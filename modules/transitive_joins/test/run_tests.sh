echo "Running all tests...";

# Switch the directory and build the library first
cd .. && make clean && make test && make install && cd test;

# Now build the tests
make clean;
make all;

pass=0;
fail=0;
skipped=0;

# Output of passed tests
groundTruth="Pass Test";
# Name of the fulfillment failure test
fulfillmentTestName="FulfillmentFail";
# Disabled Test Name Prefix
disabledTestNamePrefix="DisabledTest";
# Error messages
fulfillmentError="A TJPromise is not fulfilled in this task";

failedTests=()

for test in $(find . -type f -name '*.out')
do
    # Do not run disabled tests
    if [[ "$test" = *"$disabledTestNamePrefix"* ]]; then
        ((skipped += 1));
        continue;
    fi

    echo;
    echo "Running $test";

    # Execute the test and save the output
    testOutput="$($test 2>&1)";

    # Verify the output
    if [[ "$testOutput" = *"$groundTruth"* ]]; then
        ((pass += 1));
    else
        if [[ "$test" = *"$fulfillmentTestName"* ]]; then
            # If the test is related to TJPromise fulfillment failure, check for fulfillment assert failure too
            if [[ "$testOutput" = *"$fulfillmentError"* ]]; then
                ((pass += 1));
                continue;
            fi
        fi

        echo "Failed": $test;
        failedTests+=($test);
        ((fail += 1));
    fi
done

echo;
echo $(date);
echo "$pass passed, $fail failed, $skipped skipped"

echo;
echo "Failed tests listed below:";

for testName in "${failedTests[@]}"
do
  echo "${testName}"
done