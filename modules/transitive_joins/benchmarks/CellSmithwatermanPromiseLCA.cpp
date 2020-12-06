#include <fstream>
#include <iostream>
#include <string>

#include "TJAsyncAPI.h"

#include "hclib_cpp.h"

namespace tj = hclib::transitivejoins;

#define wrapPromiseForTransfer(promise) std::vector<tj::TJPromiseBase*> {promise}

const int32_t MATCH = 3;
const int32_t MISMATCH = -3;
const int32_t GAP_PENALTY = -2;
const char* FIRST_FILE = "./input/inputA.txt";
const char* SECOND_FILE = "./input/inputB.txt";

int32_t similarityScore(char firstChar, char secondChar) {
    return firstChar == secondChar ? MATCH : MISMATCH;
}

void readSequencesFromFile(
    const char* fileOne,
    const char* fileTwo,
    std::string& firstSeq,
    std::string& secondSeq
) {
    std::ifstream firstFileHandle(fileOne, std::ifstream::in);
    std::ifstream secondFileHandle(fileTwo, std::ifstream::in);

    std::getline(firstFileHandle, firstSeq);
    std::getline(secondFileHandle, secondSeq);
}

int main(int argc, char** argv) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        hclib::finish([=]() {
            std::string firstSeq;
            std::string secondSeq;

            readSequencesFromFile(FIRST_FILE, SECOND_FILE, firstSeq, secondSeq);

            size_t firstSeqSize = firstSeq.size();
            size_t secondSeqSize = secondSeq.size();

            printf("Finished reading sequences, sequence lengths are %lu and %lu\n", firstSeqSize, secondSeqSize);
            std::cout << "Initializing Scoring Matrix" << std::endl;

            auto scoringMatrix = new std::vector<std::vector<tj::TJPromise<int32_t>*>>();
            scoringMatrix->resize(firstSeqSize + 1);

            // Fill the col with rows
            for (size_t rowIdx = 0; rowIdx < firstSeqSize + 1; rowIdx++) {
                scoringMatrix->at(rowIdx).resize(secondSeqSize + 1);
            }

            // Fill the first column and row with dummy promises
            scoringMatrix->at(0).at(0) = tj::getNewTJPromise<int32_t>();
            auto firstPromise = scoringMatrix->at(0).at(0);
            tj::async_future(wrapPromiseForTransfer(firstPromise), [firstPromise]() {
                firstPromise->put(0);
            })->wait();

            for (size_t colIdx = 1; colIdx < secondSeqSize + 1; colIdx++) {
                scoringMatrix->at(0).at(colIdx) = tj::getNewTJPromise<int32_t>();
                auto promise = scoringMatrix->at(0).at(colIdx);
                tj::async_future(wrapPromiseForTransfer(promise), [promise]() {
                    promise->put(0);
                })->wait();
            }

            for (size_t rowIdx = 1; rowIdx < firstSeqSize + 1; rowIdx++) {
                scoringMatrix->at(rowIdx).at(0) = tj::getNewTJPromise<int32_t>();
                auto promise = scoringMatrix->at(rowIdx).at(0);
                tj::async_future(wrapPromiseForTransfer(promise), [promise]() {
                    promise->put(0);
                })->wait();
            }

            std::cout << "Finished Scoring Matrix" << std::endl;

            /* Start timing first */
            auto start = std::chrono::steady_clock::now();

            for (size_t row = 1; row < firstSeq.size() + 1; row++) {
                for (size_t col = 1; col < secondSeq.size() + 1; col++) {
                    scoringMatrix->at(row).at(col) = tj::getNewTJPromise<int32_t>();
                    auto promise = scoringMatrix->at(row).at(col);
                    tj::async(wrapPromiseForTransfer(promise), [promise, row, col, scoringMatrix, &firstSeq, &secondSeq]() {
                        auto topLeftScore = scoringMatrix->at(row - 1).at(col - 1)->wait()
                                            + similarityScore(firstSeq[row - 1], secondSeq[col - 1]);

                        int32_t currGAP = 1;
                        auto maxLeftScore = INT32_MIN;
                        int64_t currCol = col - 1;

                        while (currCol >= 0) {
                            maxLeftScore = std::max(
                                scoringMatrix->at(row).at(currCol)->wait() - currGAP,
                                maxLeftScore);

                            currCol--;
                            currGAP++;
                        }

                        currGAP = 1;
                        auto maxTopScore = INT32_MIN;
                        int64_t currRow = row - 1;

                        while (currRow >= 0) {
                            maxTopScore = std::max(
                                scoringMatrix->at(currRow).at(col)->wait() - currGAP,
                                maxTopScore);

                            currRow--;
                            currGAP++;
                        }

                        size_t newScore = std::max(
                            std::max(topLeftScore, 0),
                            std::max(maxLeftScore, maxTopScore)
                        );

                        promise->put(newScore);
                    });
                }
            }
            
            scoringMatrix->at(firstSeq.size()).at(secondSeq.size())->wait();

            /* End timing */
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start
            );

            std::cout << "Algorithm took " << duration.count() / 1000.0 << " seconds" << std::endl;

            // Free the dummy promises
            if (scoringMatrix) delete scoringMatrix;
        });
    });

    return 0;
}