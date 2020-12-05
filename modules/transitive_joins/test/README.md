# Tests

- This folder contains all of the test files.
- Use `make all` to build all test files
- Use `./TestName.out` to run each individual test
- A bash script, `run_tests.sh` is provided to execute all of the tests at once. Example output

```
Running ./PromiseLCAConvex.out

Running ./FulfillmentFail.out

Running ./PromiseFulfillment.out

Running ./FutureLCAViolation.out

Running ./AsyncTaskNodeAssociation.out

Running ./ExampleProgram.out

Running ./PromiseLCAParentChild.out

Running ./FutureLCAParentWaitsChildren.out

Running ./DoubleTransfer.out

Running ./PromiseLCAConcave.out

Running ./PromiseLCABasicViolation.out

Sat Dec 5 13:21:30 EST 2020
11 passed, 0 failed, 0 skipped

Failed tests listed below:
```