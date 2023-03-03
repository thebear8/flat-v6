# Coding Style and Guidelines
As this project is growing, the need for a clear coding standard is becoming more apparent.

## Coding
### Reduce the number of includes in header files
Reducing the number of includes in header files leads to less scope pollution, faster compile times and easier identification of dependencies. Forward declaring classes is very helpful in avoiding unnecessary includes.

### Use self-sufficient headers
Making header files self-sufficient where reasonably possible

### Don't try to be too clever
Writing very clever code ultimately ends up hurting you in the future, as you spend lots of time figuring out what some particularly clever piece of code you wrote a year ago does.

### Design class interfaces in a clean, easy to work with fashion
When designing an interface to some class, design it so that you are going to enjoy using that class.

### Don't follow design patterns like they are the law
Design patterns like the visitor pattern are useful, but there is noting stopping you from using your own judgement and extending or improving them where necessary. For example, the classic visitor pattern in c++ doesn't really allow for returning an arbitrary value from a function. As that is pretty useful however, it makes a lot of sense to leave the original visitor pattern behind and implement something better.

### Think about data models
Think very carefully about how you want to model data in the AST/IR. The way you model something will dictate entirely how you are going to work with that data later on.

### Seperate data and program logic clearly
Seperating data like the AST and IR from the code that uses that data cleanly is incredibly important. Not doing so is going to end up in an utter mess where it's very hard to add new functionality and maintain old functionality.

## Formatting
Code formatting is inherently a very subjective thing. What looks good to someone might look bad someone else. The following is completely based on my preferences. 

However, I find that much more important than the formatting style itself is to choose one style and enforce that style everywhere in a consistent manner.

To do so, this project uses clang-format. As the configuration options for clang-format are virtually unlimited, I will not cover all of them here. Ultimately, they don't matter very much as long as the code is consistent.

### Line breaks
There seems to be a bit of a debate about whether you should or should not wrap lines, but I found it useful to do so at 80 characters. This helps tremendously when using VSCode's split view feature, because you dont ever really have to scroll horizontally, which becomes very annoying.

### Line wrapping before braces
I tried multiple brace wrapping styles but i ultimately found Allman Style to be best suited for the project. This is because it tends to look much less crowded than other styles. In such a compiler project a lot of code inherently ends up being fairly complex. When analysing a complex piece of code, I found the additional crowdedness that comes from other brace wrapping styles a bit annoying.

Brace wrapping before lambda bodies poses an exception to the Allman Style. I find lambdas much more pleasant to look at when the opening brace of the body is on the same line as the lambda's head.