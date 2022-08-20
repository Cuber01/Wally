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

void runtimeError(uint16_t line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[line %d] Runtime Error : ", line);
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

static void defineMethod()
{
    ObjFunction* method = AS_FUNCTION(pop());
    ObjClass* klass = AS_CLASS(peek(0));

    vm.currentClosure = vm.currentEnvironment;
    method->closure = vm.currentClosure;

    tableSet(klass->methods, method->name, OBJ_VAL(method));
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

static bool call(ObjFunction* function, ObjInstance* thisValue, uint16_t argCount, uint16_t line)
{
    if(argCount != function->arity)
    {
        runtimeError(line, "Expected %d arguments but got %d.", function->arity, argCount);
        return false;
    }

    vm.ip = function->chunk.code;
    vm.currentEnvironment = newEnvironment(); // todo found the culprit

    // Define 'this' to be replaced by instance in methods
    if(thisValue != NULL)
    {
        environmentDefine(vm.currentEnvironment, vm.thisString, OBJ_VAL(thisValue), line);
    }

    vm.currentEnvironment->enclosing = function->closure;
    vm.currentFunction = function;

    return true;
}

static void callNative(Value callee, uint16_t line, uint8_t argCount)
{
    NativeFn native = AS_NATIVE(callee);
    Value result = native(argCount, line, vm.stackTop - argCount);
    pop();
    push(result);
}

static bool callValue(Value callee, uint8_t argCount, uint16_t line)
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
                bound->method->calledFromEnvironment = vm.currentEnvironment;

                return call(bound->method, bound->instance, argCount, line);
            }

            case OBJ_FUNCTION:
            {
                ObjFunction* function = AS_FUNCTION(callee);

                function->calledFromFunction = vm.currentFunction;
                function->calledFromIp = vm.ip;
                function->calledFromEnvironment = vm.currentEnvironment;

                return call(function, NULL, argCount, line);
            }

            case OBJ_CLASS:
            {
                ObjClass* klass = AS_CLASS(callee);
                push(OBJ_VAL(newInstance(klass)));

                Value initializer;
                if (tableGet(klass->methods, vm.initString,&initializer))
                {
                    ObjFunction* init = AS_FUNCTION(initializer);

                    init->calledFromFunction = vm.currentFunction;
                    init->calledFromIp = vm.ip;
                    init->calledFromEnvironment = vm.currentEnvironment;

                    return call(AS_FUNCTION(initializer), AS_INSTANCE(peek(0)),  argCount, line);
                }

                return true;
            }

            case OBJ_NATIVE:
            {
                callNative(callee, line, argCount);
                return true;
            }

            default:
                break; // Non-callable object type.
        }

    }

    runtimeError(line, "Can only call functions and classes.");
    return false;
}

static bool bindMethod(ObjClass* klass, ObjInstance* instance, ObjString* name, uint16_t line)
{
    Value method;

    if (!tableGet(klass->methods, name, &method))
    {
        runtimeError(line, "Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(instance, AS_FUNCTION(method));
    push(OBJ_VAL(bound));
    return true;
}

static bool invokeFromClass(ObjInstance* instance, ObjString* name, uint8_t argCount, uint16_t line)
{
    Value method;
    if (!tableGet(instance->klass->methods, name, &method))
    {
        runtimeError(line, "Undefined property '%s'.", name->chars);
        return false;
    }

    if(IS_NATIVE(method))
    {
        callNative(method, line, argCount);
        return true;
    }

    ObjFunction* callee = AS_FUNCTION(method);

    callee->calledFromFunction = vm.currentFunction;
    callee->calledFromIp = vm.ip;
    callee->calledFromEnvironment = vm.currentEnvironment;

    return call(callee, instance, argCount, line);
}

static bool invoke(ObjString* name, int argCount, uint16_t line)
{
    Value receiver = peek(argCount);

    if (!IS_INSTANCE(receiver))
    {
        runtimeError(line, "Only instances have methods.");
        return false;
    }


    return invokeFromClass(AS_INSTANCE(receiver), name, argCount, line);
}

// endregion

// region Run

static int run()
{
    uint16_t line;

    #define READ_BYTE() (*vm.ip++)
    #define READ_BYTE_NO_INCREMENT (*vm.ip)
    #define READ_SHORT() \
        (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))

    #define READ_CONSTANT() (vm.currentFunction->chunk.constants.values[READ_BYTE()])
    #define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
            { \
                runtimeError(line, "Both operands must be numbers."); \
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
        line = vm.currentFunction->chunk.lines[(int)(vm.ip - vm.currentFunction->chunk.code)];

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
                    runtimeError(line, "Operand must be a number.");
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
                    runtimeError(line, "Operands must be either two numbers or two strings.");
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

                environmentDefine(vm.currentEnvironment, name, initializer, line);

                break;
            }

            case OP_DEFINE_ARGUMENT:
            {
                ObjString* name = READ_STRING();
                Value initializer = pop();

                if(!environmentDefine(vm.currentEnvironment, name, initializer, line))
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
                        runtimeError(line, "Tried to get value of '%s', but it doesn't exist.", name->chars);
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

                if (!environmentSet(vm.currentEnvironment, name, value, line))
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

                environmentDefine(vm.currentEnvironment, function->name, OBJ_VAL(function), line);

                break;
            }

            case OP_DEFINE_CLASS:
            {
                ObjClass* klass = AS_CLASS(peek(0));

                environmentDefine(vm.currentEnvironment, klass->name, OBJ_VAL(klass), line);
                break;
            }

            case OP_INVOKE:
            {
                ObjString* method = READ_STRING();
                uint8_t argCount = READ_BYTE();

                if (!invoke(method, argCount, line))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_GET_BASE:
            {
                ObjString* name = READ_STRING();

                if(charsEqual(name->chars, "init", name->length, 4))
                {
                    runtimeError(line, "Cannot call base initializer.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                Value instanceV = peek(0);

                if(!IS_INSTANCE(instanceV))
                {
                    runtimeError(line, "Cannot use 'base' outside of a method.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(instanceV);
                ObjClass* base = (ObjClass*) instance->klass->parent;
                push(OBJ_VAL(base)); // Just to feed the pops and returns

                if(base == NULL)
                {
                    runtimeError(line, "Cannot use 'base' in a class that does not inherit from another.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                if (!bindMethod(base, instance, name, line))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_GET_PROPERTY:
            {
                if (!IS_INSTANCE(peek(0)))
                {
                    runtimeError(line, "Only instances have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();

                Value value;

                if (!tableGet(instance->fields, name, &value))
                {
                    if (!bindMethod(instance->klass, instance, name, line))
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
                    runtimeError(line, "Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(instanceVal);

                tableSet(instance->fields, fieldName, initializer);
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

                callValue(callee, argCount, line);

                break;
            }

            case OP_INHERIT:
            {
                Value base = peek(0);
                ObjClass* child = AS_CLASS(peek(1));

                if (!IS_CLASS(base))
                {
                    runtimeError(line, "Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                // Just for convenience.
                ObjClass* parent = AS_CLASS(base);

                tableAddAll(parent->methods,child->methods);

                child->parent = (struct ObjClass*) parent;

                pop(); // Parent.
                break;
            }

            case OP_DEFINE_METHOD:
                defineMethod();
                break;

            case OP_BUILD_LIST:
            {
                ObjWList* list = newWList();
                int count = READ_BYTE();

                push(OBJ_VAL(list));

                for (int i = count; i > 0; i--)
                {
                    addWList(list, peek(i));
                }

                pop();

                while (count > 0)
                {
                    pop();
                    count--;
                }

                push(OBJ_VAL(list));

                break;
            }

            case OP_LIST_STORE:
            {
                uint index = AS_NUMBER(pop());
                Value value = pop();
                ObjWList* list = AS_LIST(pop());

                storeWList(list, value, index);

                break;
            }

            case OP_LIST_GET:
            {
                uint index = AS_NUMBER(pop());
                ObjWList* list = AS_LIST(pop());

                push(indexFromWList(list, index));

                break;
            }

            case OP_RETURN:
            {
                ObjFunction* oldFunction = vm.currentFunction;

                if(oldFunction->calledFromFunction == NULL)
                {
                    // printVariables(vm.currentEnvironment);
                    // collectGarbage();
                    return INTERPRET_OK;
                }

                if(oldFunction->type == TYPE_METHOD)
                {
                    pop(); // Instance.
                }

                vm.currentFunction = oldFunction->calledFromFunction;
                vm.ip = oldFunction->calledFromIp;
                freeEnvironment(vm.currentEnvironment);
                vm.currentEnvironment = oldFunction->calledFromEnvironment;

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
    vm.strings = ALLOCATE_TABLE();
    initTable(vm.strings);
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

    defineCore(vm.nativeEnvironment->values);

    vm.objects = NULL;
}

void freeVM()
{
    vm.initString = NULL;
    vm.thisString = NULL;

    freeEnvironmentsRecursively(vm.currentEnvironment);
    freeEnvironment(vm.nativeEnvironment);
    freeTable(vm.strings);
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

    function->closure = NULL;

    call(function, NULL, 0, 0);

    gcStarted = true;
    return run();
}

// endregion