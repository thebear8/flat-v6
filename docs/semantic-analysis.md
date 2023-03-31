# Semantic Analysis

## Call Target Resolution

### Step 1: Constraint Condition Lookup
This step checks if we have a constraint condition with correct name and parameters.

Correct parameters means an exact type match in this step.

### Step 2: Function Template Lookup
In this step, we search for all function templates, which have the correct name and are compatible with the given args. 

Parameter and argument types are compatible if:
- The parameter type and the argument type are the same
- The parameter type is a generic placeholder type (e.g. T) and the argument type is any other type (including concrete *and* generic types, arguments of generic type are possible in generic functions).

Then we filter out all candidates for which the requirements are not fullfilled.

To continue, we sort all candidates in ascending order by the number of generic type substitutions we have to do in order for the candidate to be compatible. This is done to ensure that we prioritize specializations of a function template over the generic version.

Following that, the only thing we still have to do is to make sure we do not have more than one candidate with the same number of substitutions.

### Step 3: Make sure we only have one candidate at the end
If both previous steps yielded a candidate, we cannot be sure which candidate is the right one. Therefore, we have to report an ambiguous function call error to the user if this is the case.

If we only have one candidate left, we are done.