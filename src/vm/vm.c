#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "vm.h"
#include "disassembler.h"
#include "parser.h"
#include "object.h"
#include "memory.h"
#include "emitter.h"
#include "garbage_collector.h"
#include "core.h"

VM vm;

// region Error

static void resetStack()
{
    // This is the equivalent of: vm.stackTop = &vm.stack[0]
    vm.stackTop = vm.stack;
}

void runtimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    resetStack();
}

// endregion

// region VM Utils

void push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

// endregion

// region Runtime Utils

static void defineNative(const char* name, NativeFn function)
{
    tableDefineEntry(
            &vm.nativeEnvironment->values,
            copyString(name, (int)strlen(name)),
            OBJ_VAL((Obj*)newNative(function))
    );
}

static void defineMethod()
{
    ObjFunction* method = AS_FUNCTION(pop());
    ObjClass* klass = AS_CLASS(peek(0));
    tableSet(&klass->methods, method->name, OBJ_VAL(method));
}

static bool isFalsey(Value value)
{
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static inline ObjString* toString(Value value)
{
    if(IS_OBJ(value))
    {
        return objectToString(value);
    }
    else
    {
        return valueToString(value);
    }
}

static void concatenate()
{
    ObjString* b = toString(peek(0));
    ObjString* a = toString(peek(1));

    uint length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);

    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static bool call(ObjFunction* function, ObjInstance* thisValue, uint16_t argCount)
{
    if(argCount != function->arity)
    {
        runtimeError("Expected %d arguments but got %d.", function->arity, argCount);
        return false;
    }

    vm.ip = function->chunk.code;
    vm.currentEnvironment = newEnvironment();

    // Define 'this' to be replaced by instance in methods
    if(thisValue != NULL)
    {
        environmentDefine(vm.currentEnvironment, vm.thisString, OBJ_VAL(thisValue));
    }

    vm.currentEnvironment->enclosing = function->closure; // TODO function closure = NULL
    vm.currentFunction = function;

    return true;
}

static bool callValue(Value callee, uint16_t argCount)
{
    if (IS_OBJ(callee))
    {
        switch (OBJ_TYPE(callee))
        {
            case OBJ_BOUND_METHOD:
            {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);

                bound->method->calledFromFunction = vm.currentFunction;
                bound->method->calledFromIp = vm.ip;

                return call(bound->method, bound->instance, argCount);
            }

            case OBJ_FUNCTION:
            {
                ObjFunction* function = AS_FUNCTION(callee);

                function->calledFromFunction = vm.currentFunction;
                function->calledFromIp = vm.ip;

                return call(function, NULL, argCount);
            }

            case OBJ_CLASS:
            {
                ObjClass* klass = AS_CLASS(callee);
                push(OBJ_VAL(newInstance(klass)));

                Value initializer;
                if (tableGet(&klass->methods, vm.initString,&initializer))
                {
                    ObjFunction* init = AS_FUNCTION(initializer);

                    init->calledFromFunction = vm.currentFunction;
                    init->calledFromIp = vm.ip;

                    return call(AS_FUNCTION(initializer), AS_INSTANCE(peek(0)),  argCount);
                }

                return true;
            }

            case OBJ_NATIVE:
            {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                pop();
                push(result);
                return true;
            }

            default:
                break; // Non-callable object type.
        }

    }

    runtimeError("Can only call functions and classes.");
    return false;
}

static bool bindMethod(ObjClass* klass, ObjInstance* instance, ObjString* name)
{
    Value method;

    if (!tableGet(&klass->methods, name, &method))
    {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(instance, AS_FUNCTION(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}

static bool invokeFromClass(ObjInstance* instance, ObjString* name, int argCount)
{
    Value method;
    if (!tableGet(&instance->klass->methods, name, &method))
    {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjFunction* callee = AS_FUNCTION(method);

    callee->calledFromFunction = vm.currentFunction;
    callee->calledFromIp = vm.ip;

    return call(callee, instance, argCount);
}

static bool invoke(ObjString* name, int argCount)
{
    Value receiver = peek(argCount);

    if (!IS_INSTANCE(receiver))
    {
        runtimeError("Only instances have methods.");
        return false;
    }

    return invokeFromClass(AS_INSTANCE(receiver), name, argCount);
}

// endregion

// region Run

static int run()
{

    #define READ_BYTE() (*vm.ip++)
    #define READ_BYTE_NO_INCREMENT (*vm.ip)
    #define READ_SHORT() \
        (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))

    #define READ_CONSTANT() (vm.currentFunction->chunk.constants.values[READ_BYTE()])
    #define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
            { \
                runtimeError("Both operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push(valueType(a op b)); \
        } while (false)

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    for (;;)
    {
        #ifdef DEBUG_TRACE_EXECUTION
        disassembleInstruction(&vm.currentFunction->chunk,
        (int)(vm.ip - vm.currentFunction->chunk.code));

        // Print the whole stack
        printf("        |  ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        putchar('\n');

        #endif

        switch (READ_BYTE())
        {

            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NULL:   push(NULL_VAL);        break;
            case OP_TRUE:   push(BOOL_VAL(true));  break;
            case OP_FALSE:  push(BOOL_VAL(false)); break;

            case OP_POP: pop(); break;

            case OP_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_SWITCH_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(a);
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_NOT_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(!valuesEqual(a, b)));
                break;
            }

            case OP_GREATER:        BINARY_OP(BOOL_VAL, >);  break;
            case OP_LESS:           BINARY_OP(BOOL_VAL, <);  break;
            case OP_GREATER_EQUAL:  BINARY_OP(BOOL_VAL, >=); break;
            case OP_LESS_EQUAL:     BINARY_OP(BOOL_VAL, <=); break;

            case OP_NEGATE:
                if (!IS_NUMBER(peek(0)))
                {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;

            case OP_NOT:
            {
                push(BOOL_VAL(isFalsey(pop())));
                break;
            }

            case OP_ADD:
            {
                if (IS_STRING(peek(0)) || IS_STRING(peek(1)))
                {
                    concatenate();
                }
                else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
                {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                }
                else
                {
                    runtimeError("Operands must be either two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;

            case OP_DEFINE_VARIABLE:
            {
                Value initializer = pop();
                ObjString* name = READ_STRING();

                environmentDefine(vm.currentEnvironment, name, initializer);

                break;
            }

            case OP_DEFINE_ARGUMENT:
            {
                ObjString* name = READ_STRING();
                Value initializer = pop();

                if(!environmentDefine(vm.currentEnvironment, name, initializer))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_GET_VARIABLE:
            {
                ObjString* name = READ_STRING();
                Value value;

                if (!environmentGet(vm.currentEnvironment, name, &value))
                {
                    if(!environmentGet(vm.nativeEnvironment, name, &value))
                    {
                        runtimeError("Tried to get value of '%s', but it doesn't exist.", name->chars);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }

                push(value);
                break;
            }

            case OP_SET_VARIABLE:
            {
                Value value = pop();
                ObjString* name = READ_STRING();

                if (!environmentSet(vm.currentEnvironment, name, value))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(value);

                break;
            }

            case OP_DEFINE_FUNCTION:
            {
                ObjFunction* function = AS_FUNCTION(pop());
                vm.currentClosure = vm.currentEnvironment;
                function->closure = vm.currentClosure;

                environmentDefine(vm.currentEnvironment, function->name, OBJ_VAL(function));

                break;
            }

            case OP_DEFINE_CLASS:
            {
                ObjClass* klass = AS_CLASS(peek(0));

                environmentDefine(vm.currentEnvironment, klass->name, OBJ_VAL(klass));
                break;
            }

            case OP_INVOKE:
            {
                ObjString* method = READ_STRING();
                uint8_t argCount = READ_BYTE();

                if (!invoke(method, argCount))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_GET_BASE:
            {
                ObjString* name = READ_STRING();
                Value instanceV = peek(0);

                if(!IS_INSTANCE(instanceV))
                {
                    runtimeError("Cannot use 'base' outside of a method.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(instanceV);
                ObjClass* base = (ObjClass*) instance->klass->parent;

                if(base == NULL)
                {
                    runtimeError("Cannot use 'base' in a class that does not inherit from another.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                if (!bindMethod(base, instance, name))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_GET_PROPERTY:
            {
                if (!IS_INSTANCE(peek(0)))
                {
                    runtimeError("Only instances have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();

                Value value;

                if (!tableGet(&instance->fields, name, &value))
                {
                    if (!bindMethod(instance->klass, instance, name))
                    {
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    break;
                }

                pop(); // Instance.
                push(value);
                break;

            }

            case OP_SET_PROPERTY:
            {
                ObjString* fieldName = READ_STRING();
                Value initializer = pop();
                Value instanceVal = pop();

                if (!IS_INSTANCE(instanceVal))
                {
                    runtimeError("Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(instanceVal);

                tableSet(&instance->fields, fieldName, initializer);
                push(initializer);
                break;
            }

            case OP_SCOPE_START:
            {
                Environment* enclosing = vm.currentEnvironment;

                vm.currentEnvironment = newEnvironment();
                vm.currentEnvironment->enclosing = enclosing;
                break;
            }

            case OP_SCOPE_END:
            {
                Environment* old = vm.currentEnvironment;
                vm.currentEnvironment = vm.currentEnvironment->enclosing;
                freeEnvironment(old);
                break;
            }

            case OP_JUMP_IF_FALSE:
            {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) vm.ip += offset;
                break;
            }

            case OP_JUMP_IF_TRUE:
            {
                uint16_t offset = READ_SHORT();
                if (!isFalsey(peek(0))) vm.ip += offset;
                break;
            }

            case OP_JUMP:
            {
                uint16_t offset = READ_SHORT();
                vm.ip += offset;
                break;
            }

            case OP_LOOP:
            {
                uint16_t offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }

            case OP_TERNARY:
            {
                Value elseBranch = pop();
                Value thenBranch = pop();
                Value condition = pop();

                if(isFalsey(condition))
                {
                    push(elseBranch);
                }
                else
                {
                    push(thenBranch);
                }

                break;
            }

            case OP_CALL:
            {
                Value callee = pop();
                uint8_t argCount = READ_BYTE();

                callValue(callee, argCount);

                break;
            }

            case OP_INHERIT:
            {
                Value base = peek(0);
                ObjClass* child = AS_CLASS(peek(1));

                if (!IS_CLASS(base))
                {
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                // Just for convenience.
                ObjClass* parent = AS_CLASS(base);

                tableAddAll(&parent->methods,&child->methods);
                child->parent = (struct ObjClass*) parent;

                pop(); // Subclass.
                break;
            }

            case OP_DEFINE_METHOD:
                defineMethod();
                break;

            case OP_RETURN:
            {
                ObjFunction* oldFunction = vm.currentFunction;

                if(oldFunction->calledFromFunction == NULL)
                {
                    // printVariables(vm.currentEnvironment);
                    collectGarbage();
                    return INTERPRET_OK;
                }

                vm.currentFunction = oldFunction->calledFromFunction;
                vm.ip = oldFunction->calledFromIp;

                break;
            }
        }
    }

    #undef READ_SHORT
    #undef READ_STRING
    #undef BINARY_OP
    #undef READ_CONSTANT
    #undef READ_BYTE
    #undef READ_BYTE_NO_INCREMENT
}

// endregion

// region Main

void initVM()
{
    resetStack();
    initTable(&vm.strings);
    vm.currentEnvironment = newEnvironment();
    vm.nativeEnvironment = newEnvironment();
    vm.currentClosure = vm.currentEnvironment;

    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;

    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;

    // NULLs are needed to make sure the garbage collector doesn't free them
    vm.initString = NULL;
    vm.initString = copyString("init", 4);
    vm.thisString = NULL;
    vm.thisString = copyString("this", 4);

    defineNative("print", printNative);

    vm.objects = NULL;
}

void freeVM()
{
    vm.initString = NULL;
    vm.thisString = NULL;

    freeEnvironmentsRecursively(vm.currentEnvironment);
    freeEnvironment(vm.nativeEnvironment);
    freeTable(&vm.strings);
    freeObjects();
}

int interpret(const char* source)
{
    if(*source == '\0')
    {
        printf("Source is empty.");
        return INTERPRET_OK;
    }

    Node* statements = compile(source);
    if (statements == NULL) return INTERPRET_COMPILE_ERROR;

    ObjFunction* function = emit(statements);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));

    // vm.currentFunction = function;
    call(function, NULL, 0);

    gcStarted = true;
    return run();
}

// endregion