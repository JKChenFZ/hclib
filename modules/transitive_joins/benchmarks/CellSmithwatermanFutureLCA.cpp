#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include "TJAsyncAPI.h"

#include "hclib_cpp.h"

namespace tj = hclib::transitivejoins;

const int32_t MATCH = 3;
const int32_t MISMATCH = -3;
const int32_t PENALTY = -10;
const char* FIRST_FILE = "./input/inputA.txt";
const char* SECOND_FILE = "./input/inputB.txt";

typedef struct ScoringMatrixElement {
    ScoringMatrixElement() : score(0), futurePtr(nullptr) {}

    int32_t score;
    tj::TJFuture<int32_t>* futurePtr;
} ScoringMatrixElement;

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

            // First row and col are reserved for 0s
            ScoringMatrixElement** scoringMatrix = new ScoringMatrixElement*[firstSeqSize + 1];
            for (size_t row = 0; row < firstSeqSize + 1; row++) {
                scoringMatrix[row] = new ScoringMatrixElement[secondSeqSize + 1];
            }

            auto dummyFuture = tj::async_future([]() { return 0; });

            // Fill the first row and the first col with scores of 0 and a dummy future with a score of 0
            for (size_t rowIdx = 0; rowIdx < firstSeqSize + 1; rowIdx++) {
                scoringMatrix[rowIdx][0].score = 0;
                scoringMatrix[rowIdx][0].futurePtr = dummyFuture;
            }

            for (size_t colIdx = 0; colIdx < secondSeqSize + 1; colIdx++) {
                scoringMatrix[0][colIdx].score = 0;
                scoringMatrix[0][colIdx].futurePtr = dummyFuture;
            }

            std::cout << "Finished Scoring Matrix" << std::endl;
            /* Start timing first */                                                                                                               
            auto start = std::chrono::steady_clock::now();

            for (size_t row = 1; row < firstSeq.size() + 1; row++) {
                for (size_t col = 1; col < secondSeq.size() + 1; col++) {
                    scoringMatrix[row][col].futurePtr = tj::async_future(
                        [row, col, scoringMatrix, &firstSeq, &secondSeq]() {
                            int32_t topLeftScore = scoringMatrix[row - 1][col - 1].futurePtr->wait() + similarityScore(firstSeq[row - 1], secondSeq[col - 1]);
                            
                            int32_t currGAP = 1;
                            auto maxLeftScore = INT32_MIN;
                            int64_t currCol = col - 1;

                            while (currCol >= 0) {
                                maxLeftScore = std::max(
                                    scoringMatrix[row][currCol].futurePtr->wait() - currGAP,
                                    maxLeftScore);

                                currCol--;
                                currGAP++;
                            }

                            currGAP = 1;
                            auto maxTopScore = INT32_MIN;
                            int64_t currRow = row - 1;

                            while (currRow >= 0) {
                                maxTopScore = std::max(
                                    scoringMatrix[currRow][col].futurePtr->wait() - currGAP,
                                    maxTopScore);

                                currRow--;
                                currGAP++;
                            }

                            int32_t newScore = std::max(
                                std::max(topLeftScore, 0),
                                std::max(maxLeftScore, maxTopScore)
                            );

                            /* Update the scoringMatrix */
                            scoringMatrix[row][col].score = newScore;
                            return newScore;
                        }
                    );
                }
            }

            scoringMatrix[firstSeq.size()][secondSeq.size()].futurePtr->wait();

            /* End timing */                                                                                                                        \
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(                                                                  \
                std::chrono::steady_clock::now() - start                                                                                            \
            );                                                                                                                                      \
            std::cout << "Algorithm took " << duration.count() / 1000.0 << " seconds" << std::endl;
        });
    });

    return 0;
}