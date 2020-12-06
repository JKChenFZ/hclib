#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "TJAsyncAPI.h"

#include "hclib_cpp.h"

namespace tj = hclib::transitivejoins;

void merge(std::vector<size_t>& nums, size_t l, size_t m, size_t r) {
    size_t n1 = m - l + 1;
    size_t n2 = r - m;
 
    // Create temp arrays
    std::vector<size_t> L;
    std::vector<size_t> R;
 
    // Copy data to temp arrays L[] and R[]
    for (int i = 0; i < n1; i++)
        L.push_back(nums[l + i]);
    for (int j = 0; j < n2; j++)
        R.push_back(nums[m + 1 + j]);
 
    // Merge the temp arrays back into arr[l..r]
    // Initial index of first subarray
    size_t i = 0;
 
    // Initial index of second subarray
    size_t j = 0;
 
    // Initial index of merged subarray
    size_t k = l;
 
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            nums[k] = L[i];
            i++;
        } else {
            nums[k] = R[j];
            j++;
        }
        k++;
    }
 
    // Copy the remaining elements of
    // L[], if there are any
    while (i < n1) {
        nums[k] = L[i];
        i++;
        k++;
    }
 
    // Copy the remaining elements of
    // R[], if there are any
    while (j < n2) {
        nums[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(std::vector<size_t>& nums, size_t l, size_t r) {
    if ((r - l) <= 1000) {
        // Directly sort and return
        std::vector<size_t> temp;
        for (size_t i = l; i <= r; i++) {
            temp.push_back(nums[i]);
        }

        std::sort(temp.begin(), temp.end());

        // Copy the values back into the nums
        for (size_t i = l; i <= r; i++) {
            nums[i] = temp[i - l];
        }

        return;
    }

    size_t m = (l + r - 1) / 2;
    auto leftPromise = tj::getNewTJPromise<bool>();
    auto rightPromise = tj::getNewTJPromise<bool>();

    // Transfer the promise to the left half, the right half will wait on it
    auto lHandle = tj::async_future(
        std::vector<tj::TJPromiseBase*>{leftPromise},
        [&nums, l, m, leftPromise]() {
            mergeSort(nums, l, m);
            leftPromise->put(true);
        }
    );

    // Transfer the promise to the right half, parent will wait on it
    auto rHandle = tj::async_future(
        std::vector<tj::TJPromiseBase*>{rightPromise},
        [&nums, m, r, leftPromise, rightPromise]() {
            // Wait for the completion of the left half
            assert(leftPromise->wait());
            mergeSort(nums, m + 1, r);
            rightPromise->put(true);
        }
    );

    assert(rightPromise->wait());
    merge(nums, l, m, r);
}

int main(int argc, char** argv) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        // Generate an unsorted array ahead of time
        size_t size = atoi(argv[1]);
        std::vector<size_t> numbers;
        numbers.reserve(size);

        for (size_t i = 0; i < size; i++) {
            numbers.push_back(size - i);
        }

        std::cout << "Sorting " << numbers.size() << " numbers" << std::endl;
        auto start = std::chrono::steady_clock::now();

        auto handle = tj::async_future([&numbers]() {
            mergeSort(numbers, 0, numbers.size() - 1);
        });

        handle->wait();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start
        );

        std::cout << "Algorithm took " << duration.count() / 1000.0 << " seconds" << std::endl;

        // Verify that numbers are actually sorted
        for (size_t i = 1; i < numbers.size(); i++) {
            assert(numbers[i] >= numbers[i - 1]);
        }
    });

    return 0;
}