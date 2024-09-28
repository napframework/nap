<br>
<p align="center">
  <img width=384 src="https://download.nap-labs.tech/identity/svg/logos/nap_logo_blue.svg">
</p>

C++ Style Guide
=======================
Loosely based on the Google C++ style guide

*	[Header Files](#header-files)
*	[Scoping](#scoping)
	*	[Namespaces](#namespaces)
	*	[Static and Global Functions](#static-and-global-functions)
	*	[Local Variables](#local-variables)
	*	[Static and Global Variables](#static-and-global-variables)
*	[Classes](#classes)
	*	[Constructors](#constructors)
	*	[Destructors](#destructors)
	*	[Copy](#copy)
	*	[Move](#move)
	*	[Inheritance](#inheritance)
	*	[Operator Overloading](#operator-overloading)
	*	[Declaration Order](#declaration-order)
*	[Functions](#functions)
	*	[Write Short Functions](#write-short-functions)
*	[C++ Features](#c++-features)
	*	[Exceptions](#exceptions)
	*	[Casting](#casting)
	*	[Const](#const)
	*	[Integer Types](#integer-types)
	*	[Macros](#macros)
	*	[Use nullptr](#use-nullptr)
	*	[Auto](#auto)
	*	[Lambdas](#lambdas)
	*	[Aliases](#aliases)
*	[Naming](#naming)
	*	[General Naming Rules](#general-naming-rules)
	*	[File Names](#file-names)
	*	[Type Names](#type-names)
	*	[Variable Names](#variable-names)
	*	[Function Names](#function-names)
	*	[Namespace Names](#namespace-names)
	*	[Enumerator Names](#enumerator-names)
	*	[Macro Names](#macro-names)
*	[Comments](#comments)
	*	[Comment Style](#comment-style)
	*	[Class Comments](#class-comments)
	*	[Function Comments](#function-comments)
	*	[Function Definitions](#function-definitions)
	*	[Variable Comments](#variable-comments)
	*	[Line Comments](#line-comments)
	*	[TODO comments](#todo-comments)
*	[Formatting](#formatting)
	*	[Line Length](#line-length)
	*	[Spaces vs Tabs](#spaces-vs-tabs)
	*	[Function Declarations and Definitions](#function-declarations-and-definitions)
	*	[Function Calls](#function-calls)
	*	[Conditionals](#conditionals)
	*	[Switch Statements](#switch-statements)
	*	[Loops](#loops)
	*	[Pointer and Reference Expressions](#pointer-and-reference-expressions)
	*	[Variable and Array initializations](#variable-and-array-initializations)
	*	[Class and Struct Format](#class-and-struct-format)
	*	[Namespace Formatting](#namespace-formatting)

Header Files
-----------------------

- In general, every `.cpp` file should have an associated `.h` file. There are some common exceptions, such as unittests and small `.cpp` files containing just a `main()` function
- Try to make every header self contained, ie: can be included without having to rely on other headers.
- A header should have header guards and include all the other headers it needs
- Functions are declared in the header and defined in the cpp file. One liners are an exception to this rule, ie, can be declared and defined on the same line, but should not exceed a single line. Template definitions are defined at the end of a file or in a separate include file, ie, `_include.hpp`
- At the beginning of a header file: don't use `#ifndef` `#define`, use: `#pragma` once
- Try to avoid forward declarations, unless it's absolutely necessary to avoid cyclic dependencies. If you have to use forward declarations to avoid a cyclic dependency it's probably best to rethink your code design.
- Don't use inline, have faith in your compiler
- Use quotes `""` for local includes and `<>` for external includes. Say you have a file with the following path: `c:/projectx/include/base.h`, when you want to include `base.h` in a file that resides in the same folder, use quotes, this makes `base.h` local to that file. If the file that includes `base.h` resides in a different directory (within the same or external project), use brackets.
- Start with local includes followed by external includes

Scoping
-----------------------

### Namespaces

With few exceptions, place code in the `nap` namespace. Every namespace should have unique name based on the project. Using namespaces in `.cpp` files is encouraged. Do not use `using` directives (e.g. `using namespace foo`). If so, only in .cpp files.

- Do not use inline namespaces
- Do not use namespace aliases: `namespace foo_bar = ::foo`

### Static and Global Functions
- Prefer placing nonmember functions in a namespace.
- Try to avoid global functions
- Group functions using a namespace instead of a class as if it were a namespace
- Global functions should start with a lowerCase word: `std::string toString<T>(const T& value)`
- Static members of a class should be closely related to instances of that class
- Static members of a class start with a lowercase word: `ResourceManager::registerCreateFunctions()`
- If you define a non member function and it's only needed in it's .cpp file, use static linkage, ie: `static int foo()` to limit it's scope.

### Local Variables
- Place a function's variables in the narrowest scope possible, and initialize variables in the declaration
- Declare variables as close to the first use as possible, except in loops:
```
Foo f; // My ctor and dtor get called once each.
for (int i = 0; i < 100; ++i)
	f.doSomething(i);
```
- Prefer initialization using braces: vector<int> v = { 1, 2 };

### Static and Global Variables
- Variables of class type with static storage duration are forbidden: they cause hard-to-find bugs due to indeterminate order of construction and destruction. However, such variables are allowed if they are constexpr and marked inline.
- Objects with static storage duration, including global variables, static variables, static class member variables, and function static variables, must be Plain Old Data (POD): only ints, chars, floats, or pointers, or arrays/structs of POD.
- If you need a static or global variable of a class type, consider initializing a pointer (which will never be freed), from either your `main()` function or from `pthread_once()`. Note that this must be a raw pointer, not a "smart" pointer, since the smart pointer's destructor will have the
order-of-destructor issue that we are trying to avoid.
- Don't use `#define` to declare constants, use const or constexpr [instead](https://stackoverflow.com/questions/15218760/should-i-avoid-using-define-in-c-why-and-what-alternatives-can-i-use)
```
inline constexpr int maxRotations = 12
inline static constexpr int maxMidiValue = 127
```

Classes
-----------------------
With the introduction of move constructors and move assignment operators, the rules for when automatic versions of constructors, destructors and assignment operators are generated has become quite complex. Using = default and = delete makes things easier. Provide the copy and move operations if their meaning is clear to the user and the copying/moving does not incur unexpected costs. If you define a copy or move constructor, define the corresponding assignment operator, and vice-versa. If your type is copyable, do not define move
operations unless they are significantly more efficient than the corresponding copy operations. If your type is not copyable, but the correctness of a move is obvious to users of the type, you may make the type move-only by defining both of the move operations. Use the new C++ 11 = default and = delete keywords to create or remove the default:

- Constructor
- Destructor
- Copy Constructor
- Copy Assignment Operator
- Move Constructor
- Move Assignment Operator

### Constructors
- Avoid virtual method calls in constructors
- Initialize member variables using the `{ }` in the header
- Use the `=default` directive to add a default constructor

### Destructors
- Always make the base class constructors virtual
- If no specific behaviour is necessary, declare the destructor to be `=default`

### Copy
C++ defines it's own copy constructor and assignment operator if not defined explicitly. Try to be as obvious as possible by using the =default or =delete keywords to either allow, disallow copy functionality. Implement your own if the object manages some dynamic allocated memory or has pointer variables and you want to be able to copy it. 

- When defining a copy constructor, also define a copy assignment operator.

### Move 
C++ defines its own move constructor and assignment operator in a very limited set of circumstances. For more information read up on it over here:
http://en.cppreference.com/w/cpp/language/move_constructor. It's never safe to assume that the move operator isn't automatically implemented, it's therefore wise to show your intention using the `=default` or `=delete` keywords.

- When defining a move constructor, also define a move assignment operator.

### Inheritance
- Composition is often more appropriate than inheritance. When using inheritance, make it public.
- Try to avoid multiple inheritance

### Operator Overloading
Operator overloading can make code more concise and intuitive by enabling user-defined types to behave the same as built-in types. Overloaded operators are the idiomatic names for certain operations (e.g. `==`, `<`, `=`, and `<<`), and adhering to those conventions can make user-defined types more readable and enable them to interoperate with libraries that expect those names. 

- Define overloaded operators only if their meaning is obvious, unsurprising, and consistent with the corresponding built-in operators. For example, use `|` as a bitwise- or logical-or, not as a shell-style pipe.

### Declaration Order
Use the specified order of declarations within a class: public: before private:, methods before data members (variables), etc. Your class definition should start with its public: section, followed by its protected: section and then its private: section. If any of these sections are empty, omit them. 
Within each section, the declarations generally should be in the followingorder:

- Using-declarations, Typedefs and Enums
- Constants (static const data members)
- Constructors and assignment operators
- Destructor
- Methods, including static methods
- Data Members (except static const data members)

Functions
-----------------------
Parameter Ordering

- When defining a function, parameter order is: inputs, then outputs.
- Outputs are prefixed with `out`:

```
bool validate(const std::string& license, utility::errorState& outError)
```

Parameters to C/C++ functions are either input to the function, output from the function, or both. Input parameters are usually values or const references, while output and input/output parameters will be non-const pointers. When ordering function parameters, put all input-only parameters before any output parameters. In particular, do not add new parameters to the end of the function just because they are new; place new input-only parameters before the output parameters.

### Write Short Functions

We recognize that long functions are sometimes appropriate, so no hard limit is placed on functions length. If a function exceeds about 40 lines, think about whether it can be broken up without harming the structure of the program.

Even if your long function works perfectly now, someone modifying it in a few months may add new behavior. This could result in bugs that are hard to find. Keeping your functions short and simple makes it easier for other people to read and modify your code.

- const arguments passed by reference are always non modifiable input arguments 
	- `const float& intensity`
- Arguments passed by reference are modifiable and should be preceded by `out` 
	- `float& outIntensity`

C++ Features
-----------------------

### Exceptions
- We do not use exceptions. When a third party dependency throws an exception your are allowed to catch it. Keep the error handling code as short and local as possible. Only catch exceptions in `.cpp` files. 


### Casting
- Be as explicit as possible, something like this is not ok and most compilers will throw a warning: 
	- `float f(0.0f); int x = f;`
- Instead use: 
	- `float f(0.0f); int x = static_cast<int>(f);`
- Use C++-style casts like `static_cast<float>(double_value)` as much as possible
- Try to avoid c style casts such as `(int)3.5f` or `int(3.5f)`
- Never use `dynamic_cast`
- Try to avoid `reinterpret_cast`

The problem with C casts is the ambiguity of the operation; sometimes you are doing a conversion (e.g., (int)3.5) and sometimes you are doing a cast (e.g., (int)"hello"). Brace initialization and C++ casts can often help avoid this ambiguity. Additionally, C++ casts are more visible when searching for them.

For more information: http://stackoverflow.com/questions/103512/in-c-why-use-static-castintx-instead-of-intx

### Const
- Use const whenever it makes sense
- We encourage the use of constexpr (C++ 11)
- We encourage putting const first, so:
	-NOT: `int const * foo`
 	-BUT: `const int * foo`

Some variables can be declared constexpr to indicate the variables are true constants, i.e. fixed at compilation/link time. Some functions and   constructors can be declared constexpr which enables them to be used in defining a constexpr variable. Constexpr therefore defines a more robust specification of the constant parts of an interface. Use `constexpr` to specify true constants and the functions that support their definitions. Avoid 
complexifying function definitions to enable their use with constexpr. Do not use `constexpr` to force inlining.

### Integer Types
- The only actively used signed or unsigned integer type is int (compiler specific, minimum of 32 bits)
- Use standard library <stdint.h> integer types for different sizes of int
- NAP defines its own set of integer types based on the types in <stdint.h>
- Never use short, long etc.
- When iterating, use `size_t` or `auto`. This helps avoid platform specific warnings

### Macros
- Try to avoid them as much as possible
- Prefer inline functions, enums and const variables to macros
- Don't rely on macros to define pieces of a C++ API
- Don't use macros to store a constant, use a const variable

### Use nullptr
- Always use nullptr, never use NULL
- Use 0 for integers and 0.0 for reals

### Auto
Auto is permitted, for local variables only, when it increases readability, particularly as described below. Do not use auto for file-scope or namespace-scope variables, or for class members. Never initialize an auto-typed variable with a braced initializer list. 

Programmers have to understand the difference between `auto` and `const auto&` or they'll get copies when they didn't mean to.

- We encourage the use of auto where the type doesn't aid in clarity for the reader
	- `for ( const auto& v : vector )` 
	- `auto iterator = std::find_if(vector, ..)`
- Use type declarations when it helps readability
	- `for(size_t i=0; i<100; ++i)`
	- `float v = static_cast<float>(d)`

### Lambdas
- Use lambda expressions where appropriate.
- Prefer explicit captures when the lambda will escape the current scope.

Instead of: 
```
Foo foo;
executor->Schedule([&]
{
	Frobnicate(foo);
})
```

Use: 
```
Foo foo;
executor->Schedule([&foo]
{
	Frobnicate(foo);
})
```

### Aliases
- Use `using` instead of `typedef`

Try to limit the number of aliases in the public (header) scope. This will make them available to users of the API and creates unnecessary clutter. Only put it in your public API is you intend it to be used by your clients.

Naming
-----------------------
The most important consistency rules are those that govern naming. The style of a name immediately informs us what sort of thing the named entity is: a type, a variable, a function, a constant, a macro, etc., without requiring us to search for the declaration of that entity. The pattern-matching engine in our brains relies a great deal on these naming rules. Naming rules are pretty arbitrary, but we feel that consistency is more important than individual
preferences in this area, so regardless of whether you find them sensible or not, the rules are the rules.

Tips:
- Functions and methods should be verbs, describing an action.
- Variables should be nouns, representing state.
- Classes are nouns as well, describing a well defined concept

### General Naming Rules
- Names should be descriptive; avoid abbreviation.
- Do not worry about saving horizontal space, it's more important to make your code understandable
- Do not use abbreviations that are ambiguous to readers outside of the project
- Do not abbreviate by deleting letters within a word 

Good:
```
int price_count_reader; 	// No abbreviation.
int num_errors; 			// "num" is a widespread convention.
int num_dns_connections; 	// Most people know what "DNS" stands for.
```

Bad:
```
int n; 					// Meaningless.
int nerr; 				// Ambiguous abbreviation.
int n_comp_conns; 		// Ambiguous abbreviation.
int wgc_connections; 	// Only your group knows what this stands for.
int pc_reader; 			// Lots of things can be abbreviated "pc".
int cstmr_id; 			// Deletes internal letters.
```

### File Names
Should all be lowercase and preferably not contain any underscores or dashes

- attributes.h
- component.h
- rectangle.h
- numerictypes.h

Try to make your filenames very descriptive. For example, use: `httpclientlogs.h` rather than `logs.h`


### Type Names
Start with a capital letter and have a capital letter for each new work, with no underscores, ie:

- MyNewClass
- MyNewEnum
- MyNewStruct

Types include:

- Classes
- Structs
- Enums
- Aliases
- Template Parameters

### Variable Names
The names of class data members are all upper camelcase, preceded by the letter 'm':
```
float mIntensity; 	//< Public member
bool mCached;      	//< Private member
float mTime;		//< Private member
```

Struct members are all public and follow the class variable naming convention. Variables in function scope are all lowercase, with underscores between words.
```
int current_cycle;
vec3f new_point_position;
```

Static variables are upper camelcase
```
std::string DefaultOperatorName;
```

### Function Names
Function names are camelcase
```
getName()
getCount()
setCount()
updateCache()
```

Global functions are camelcase, also when declared in a namespace
```
smoothStep()
composeMatrix()
```

Static functions are camelcase
```
registerType()
getRegisteredTypes()
```

### Namespace Names
- Namespace names are all lower case
- Top level names are based on the project name
- Avoid nested namespaced that match well known top level namespaces

### Enumerator Names
Start with the letter 'E' and are always based on the C++ 11 enumerator classes:
```
enum class EDay : int
{
	Monday		= 1,
	Tuesday		= 2,
	Wednesday	= 3,
	Thursday	= 4,
	Friday		= 5,
	Saturday	= 6,
	Sunday		= 0
};
```

### Macro Names
If you define one use all uppercase with underscores between words:
```
MY_HORRIBLE_MACRO
RTTI_DEFINE
RTTI_DECLARE
```

Comments
-----------------------
Though a pain to write, comments are absolutely vital to keeping our code readable. The following rules describe what you should comment and where. But remember: while comments are very important, the best code is self-documenting. Giving sensible names to types and variables is much better than using obscure names that you must then explain through comments. When writing your comments, write for your audience: the next contributor who will need to understand your code. Be generous — the next one may be you!

### Comment Style
Use either the `//` or `/** */` syntax, as long as you are consistent.

### Class Comments
Every class declaration should have an accompanying comment that describes what it is and how it should be used! 

The class comment should provide the reader with enough information to know how and when to use the class, as well as any additional considerations necessary to correctly use the class. Document the synchronization assumptions the class makes, if any. If an instance of the class can be accessed by multiple threads, take extra care to document the rules and invariants surrounding multithreaded use. 

The class comment is often a good place for a small example code snippet demonstrating a simple and focused usage of the class.

When sufficiently separated (e.g. .h and .cc files), comments describing the use of the class should go together with its interface definition; comments about the class operation and implementation should accompany the implementation of the class's methods.

```
/**
 * Receives and responds to client messages over a web socket and can be used to send a reply. 
 * The server converts raw messages and connection updates from a nap::WebSocketServerEndPoint 
 * into web-socket events that are forwarded to the running application. 
 * Events are generated on a background thread and consumed on the main thread on update(). 
 * Use a nap::WebSocketComponent to receive and react to client web-socket events in your application.
 */
class NAPAPI WebSocketServer : public IWebSocketServer
{}
```

### Function Comments
Almost every function declaration should have comments immediately preceding it that describe what the function does and how to use it. These comments may be omitted only if the function is simple and obvious (e.g. simple accessors for obvious properties of the class). These comments should be descriptive ("Opens the file") rather than imperative ("Open the file"); the comment describes the function, it does not tell the function what to do. In general, these comments do not
describe how the function performs its task. Instead, that should be left to comments in the
function definition. 

Use the Javadoc style decorators to explain input parameters and return values: 

- `@param` for input and output parameters
- `@return` for the return value

```
/**
 * Converts a point in object space to world space using the given object to world matrix.
 * @param point the point location in object space
 * @param objectToWorldMatrix local to world transformation matrix
 * @return point location in world space
 */
glm::vec3 NAPAPI objectToWorld(const glm::vec3& point, const glm::mat4x4& objectToWorldMatrix);
```

Types of things to mention in comments at the function declaration:

- What the inputs and outputs are.
- For class member functions: whether the object remembers reference arguments beyond the duration of the method call, and whether it will free them or not.
- If the function allocates memory that the caller must free.
- Whether any of the arguments can be a null pointer.
- If there are any performance implications of how a function is used.
- If the function is re-entrant. What are its synchronization assumptions?

If the function is complex and needs a lot of documentation, use the Class style comments to explain functionality. Otherwise use the double slashes `//` preceding the function or, if it's a one-liner, place it after the the function declaration

### Function Definitions
If there is anything tricky about how a function does its job, the function definition should have an explanatory comment. For example, in the definition comment you might describe any coding tricks you use, give an overview of the steps you go through, or explain why you chose to implement the function in the way you did rather than using a viable alternative. For instance, you might mention why it must acquire a lock for the first half of the function but why it is not needed
for the second half.

Note you should not just repeat the comments given with the function declaration, in the .h file or
wherever. It's okay to recapitulate briefly what the function does, but the focus of the comments
should be on how it does it.

### Variable Comments
- The actual name of the variable should be descriptive enough to give a good idea of what the
variable is used for.
- Sometimes more comments are required
- All global variables should have a comment describing what they are and why it’s global
- Don’t state the obvious, if the code is self explanatory, don’t add a comment
- with multiple comments on subsequent lines, aligning them makes the code easier to read.
- The comment of a public member variable should always start with: `///<`
- The comment of a class member that is a property should start with: `///< Property: 'propertyname'`:

```
std::string	mID;	///< Property: 'mID' unique name of the object. Used as an identifier by the system
```

### Line Comments
- Lines that are non obvious should get a comment at the end of the line.
- Always separate these lined with the code using a minimum of 2 spaces
- If you have several comments on subsequent lines, it can often be more readable to line them up

### TODO comments
- Use TODO comments for code that is temporary, a short-term solution or not perfect

### Deprecation Comments
- Use `[[deprecated]]` to mark deprecated code

Formatting
-----------------------
Coding style and formatting are pretty arbitrary, but a project is much easier to follow if everyone uses the same style. Individuals may not agree with every aspect of the formatting rules, and some of the rules may take some getting used to, but it is important that all project contributors follow the style rules so that they can all read and understand everyone's code easily.

### Line Length
- Each line of text in your code should be at most 120 characters long
- Try to avoid breaking lines

### Spaces vs Tabs
- Use only tabs

### Function Declarations and Definitions
- Return type on the same line as function name
- Parameters on the same line if they fit
- Wrap parameters if they do not fit on a single line
- Opening brace after function declaration / definition
- Closing brace on seperate last line
- Align multiline parameters

```
ReturnType ClassName::FunctionName(Type par_name1, Type par_name2)
{
	doSomething();
	...
}
```

```
ReturnType LongClassName::ReallyReallyReallyLongFunctionName(
	Type par_name1, Type par_name2, Type par_name3)
{
	doSomething(); // 2 space indent
	...
}
```

Lambda Expressions
- Format parameters and bodies as for any other functions
- Format capture lists like other comma separated lists
- Don’t leave a space between the & and variable name for reference captures
```
// Example One
int x = 0;
auto add_to_x = [&x](int n)
{
	x += n;
};

//Example Two
set<int> blacklist = {7, 8, 9};
vector<int> digits = {3, 9, 1, 8, 4, 7, 1};
digits.erase(std::remove_if(digits.begin(), digits.end(), [&blacklist](int i)
{
	return blacklist.find(i) != blacklist.end();
}), digits.end());
```

### Function Calls
- Write a call on a single line or start on a new line properly indented and aligned
- Split multiple arguments when complex or confusing, create variables that capture that argument in a descriptive name
```
// Example One
bool result = doSomething(argument1, argument2, argument3);

// Example Two
int my_heuristic = scores[x] * y + bases[x];
bool result = doSomething(my_heuristic, x, y, z);
```

### Conditionals
- No spaces inside parenthesis
- The if and else keywords belong on a separate line
- Use braces with multiple line execution statements, otherwise not
- Always place braces on a separate line
- Short statements can be on one line
```
// Example One
if (condition)
{
	return ....
}
else
{
	do ...
	return ...
}

// Example Two
if (condition) { // Good - proper space after IF and before }
```

### Switch Statements
- You may use braces for blocks, following the conditional statement guidelines
- Braces are optional for single line expressions
- If not conditional or an enumerated value, switch statements should always have a default case
```
// Example One
switch (var)
{
case 0:
	break;
case 1:
	{
		...
		...
		break;
	}
default:
	assert(false);
	break;
}
```

### Loops
- Curly braces on separate line
- If a loop is a single statement, line is placed under loop expression
```
// Example One
for (int i = 0; i < some_number; ++i)
	printf("I love you\n");

// Example Two
for (int i = 0; i < some_number; ++i)
{
	printf("I'm a narcissist\n")
	printf("I take it back\n");
}
```

### Pointer and Reference Expressions
- No spaces around period or arrow
- Pointer operators do not have trailing spaces
- We do not use Hungarian declaration
```
x = *p;
p = &x;
x = r.y;
x = r->y;
```

### Variable and Array initializations
- Use `=`, `()` or `{}`
- Be careful with braced initialization lists when the type has a std::initializer_list constructor, which is always preferred by the compiler 
- Use parentheses instead of braces to force a non initializer_list constructor!
```
// Example One
vector<int> v(100, 1); 				///< A vector of 100 1s.
vector<int> v{100, 1}; 				///< A vector of 100, 1.
int pi(3.14); 						///< OK -- pi == 3.
int pi{3.14}; 						///< Compile error: narrowing conversion.
```

### Class and Struct Format
- In order: public, protected and private
- Declarations only in header, no implementation, except for one-liners
- Use the end of a file or a hpp file for template definitions
- Braces go on a separate line
- Column for readability are encouraged
- The public, protected and private key words should be on the same line as the class keyword.
```
// Example One
class MyClass
{
	Public:
	MyClass() = default;
	MyClass(int var);
	Virtual ~MyClass() = default;

	void someFunction();
	void someFunctionThatDoesNothing() 	{ }
	void setSomeVar(int var) 			{ mSomeVar = var; }
	Int getSomeVar() const 				{ return mSomeVar ; }

private:
	bool someInternalFunction();
	int mSomeVar = 0;
	Int mSomeOtherVar = 0;
};
```

### Namespace Formatting
The contents of namespaces are indented 1 tab for every namespace

```
// Example One
namespace nap
{
	void foo() {}
	...
}
```


