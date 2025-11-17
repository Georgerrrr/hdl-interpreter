# HDL Interpreter

My own very rubbish implementation of a hdl programming language.

## Building 

run `make` to compile the project. It has no external dependencies.

## Running 

You must pass chips that it will use as arguments:

`./hdl Or.hdl And.hdl Not.hdl`

To run a test you must pass that in through `stdin`:

```cat and.tst | ./hdl Or.hdl And.hdl Not.hdl```

## Todo 

 - Fix a bunch of stupid bugs
 - Make compilation errors more useful
 - Make the program fail more gracefully when it must 
 - Allow for live interpreting if no data is given through `stdin`
 - Clean up project
