//
// Created by amemiya7 on 2024/6/24.
//

#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include "Generator.h"
#include "Ai.h"

//TODO::modify linesWeight to scoreWeight
struct Candidate {
    double heightWeight;
    double scoreWeight;
    double holesWeight;
    double bumpinessWeight;
    double emptyLinesWeight;
    uint64_t fitness;
};

class Tuner {
public:
    void output(const Candidate &candidate) {
        std::cout << "heightWeight:" << candidate.heightWeight << ",linesWeight:" << candidate.scoreWeight
                  << ",holesWeight:" << candidate.holesWeight << ",bumpinessWeight:" << candidate.bumpinessWeight
                  << ",fitness:" << candidate.fitness
                  << ",emptyLinesWeight:" << candidate.emptyLinesWeight << '\n';
    }

    Tuner() {
        std::random_device rd;
        gen = std::mt19937(rd());
    }

    static void normalize(Candidate &candidate) {
        double norm = std::sqrt(
                candidate.heightWeight * candidate.heightWeight + candidate.scoreWeight * candidate.scoreWeight +
                candidate.holesWeight * candidate.holesWeight + candidate.bumpinessWeight * candidate.bumpinessWeight +
                candidate.emptyLinesWeight * candidate.emptyLinesWeight);
        //generate a number between 0 and 1


        candidate.heightWeight /= norm;
        candidate.scoreWeight /= norm;
        candidate.holesWeight /= norm;
        candidate.bumpinessWeight /= norm;
        candidate.emptyLinesWeight /= norm;
    }

    Candidate generateRandomCandidate() {
        Candidate candidate = {
                .heightWeight = randomDouble(0, 1),
                .scoreWeight = randomDouble(0, 1),
                .holesWeight = randomDouble(0, 1),
                .bumpinessWeight = randomDouble(0, 1),
                .emptyLinesWeight = randomDouble(0, 1),
        };
        normalize(candidate);
        return candidate;
    }

    //TODO::add log file
    static void computeFitnesses(std::vector<Candidate> &candidates, int numberOfGames, int maxNumberOfMoves) {
        uint32_t totalThreads = 8;
        std::vector<std::thread> threads(totalThreads - 1);
        int threadTask = candidates.size() / totalThreads;
        for (int i = 0; i < threads.size(); i++) {
            threads[i] = std::thread([i, totalThreads, &candidates, numberOfGames, maxNumberOfMoves] {
                for (int j = i; j < candidates.size(); j += totalThreads) {
                    Candidate &candidate = candidates[j];
                    AI ai = AI(candidate.heightWeight, candidate.scoreWeight, candidate.holesWeight,
                               candidate.bumpinessWeight, candidate.emptyLinesWeight);
                    uint64_t totalScore = 0;
                    for (int k = 0; k < numberOfGames; k++) {
//                        std::cerr << "Thread " << i << ", Candidate: " << j << ", Game: " << k << '\n';
                        Grid grid(10, 22);
                        Generator rpg{};

                        uint64_t score = 0;
                        int numberOfMoves = 0;
                        while ((numberOfMoves++) < maxNumberOfMoves && !grid.exceed()) {
                            //TODO::ai thread
                            Block workingPiece = rpg.nextBlock();
                            std::vector<Block> workingPieces = {workingPiece};
                            for (const auto &piece: rpg.seeNextBlocks(9)) workingPieces.push_back(piece);
                            auto [block, _, _grid_] = ai.best_(grid, workingPieces);
                            utility::insertBlock(grid, block);
                            score += utility::getScore(grid, block);
                        }
                        totalScore += score;
                    }
                    candidate.fitness = totalScore;
                }
            });
        }
        for (int i = totalThreads - 1; i < candidates.size(); i += totalThreads) {
            Candidate &candidate = candidates[i];
            AI ai = AI(candidate.heightWeight, candidate.scoreWeight, candidate.holesWeight,
                       candidate.bumpinessWeight, candidate.emptyLinesWeight);
            uint64_t totalScore = 0;
            for (int j = 0; j < numberOfGames; j++) {
//                std::cerr << "Thread " << 7 << ", Candidate: " << i << ", Game: " << j << '\n';
                Grid grid(10, 22);
                Generator rpg{};

                uint64_t score = 0;
                int numberOfMoves = 0;
                while ((numberOfMoves++) < maxNumberOfMoves && !grid.exceed()) {
                    Block workingPiece = rpg.nextBlock();
                    std::vector<Block> workingPieces = {workingPiece};
                    for (const auto &piece: rpg.seeNextBlocks(9)) workingPieces.push_back(piece);
                    auto [block, _, _grid_] = ai.best_(grid, workingPieces);
                    utility::insertBlock(grid, block);
                    score += utility::getScore(grid, block);
                }
                totalScore += score;
            }
            candidate.fitness = totalScore;
        }
        for (auto &thread: threads) {
            thread.join();
        }
    }

    std::pair<Candidate, Candidate> tournamentSelectPair(const std::vector<Candidate> &candidates, int ways) {
        std::vector<int> indices;
        for (int i = 0; i < candidates.size(); i++) {
            indices.push_back(i);
        }
        std::shuffle(indices.begin(), indices.end(), gen);
        /*
            Note that the following assumes that the candidates array is
            sorted according to the fitness of each individual candidates.
            Hence it suffices to pick the least 2 indexes out of the random
            ones picked.
        */
        int fittestCandidateIndex1 = -1;
        int fittestCandidateIndex2 = -1;
        for (int i = 0; i < ways; i++) {
            int selectedIndex = indices.back();
            indices.pop_back();
            if (fittestCandidateIndex1 == -1 || selectedIndex < fittestCandidateIndex1) {
                fittestCandidateIndex2 = fittestCandidateIndex1;
                fittestCandidateIndex1 = selectedIndex;
            } else if (fittestCandidateIndex2 == -1 || selectedIndex < fittestCandidateIndex2) {
                fittestCandidateIndex2 = selectedIndex;
            }
        }
        return {candidates[fittestCandidateIndex1], candidates[fittestCandidateIndex2]};
    }

    static Candidate crossOver(const Candidate &candidate1, const Candidate &candidate2) {
        double totalFitness = candidate1.fitness + candidate2.fitness;
        double coefficient1 = 0, coefficient2 = 0;
        if (totalFitness == 0) {
            coefficient1 = coefficient2 = 0.5;
        } else {
            coefficient1 = 1.0 * candidate1.fitness / totalFitness;
            coefficient2 = 1.0 * candidate2.fitness / totalFitness;
        }
        Candidate candidate = {
                .heightWeight = coefficient1 * candidate1.heightWeight +
                                coefficient2 * candidate2.heightWeight,
                .scoreWeight = coefficient1 * candidate1.scoreWeight +
                               coefficient2 * candidate2.scoreWeight,
                .holesWeight = coefficient1 * candidate1.holesWeight +
                               coefficient2 * candidate2.holesWeight,
                .bumpinessWeight = coefficient1 * candidate1.bumpinessWeight +
                                   coefficient2 * candidate2.bumpinessWeight,
                .emptyLinesWeight = coefficient1 * candidate1.emptyLinesWeight +
                                    coefficient2 * candidate2.emptyLinesWeight,
        };
        normalize(candidate);
        return candidate;
    }

    void mutate(Candidate &candidate) {
        double quantity = randomDouble(-0.2, 0.2);
        switch (randomInteger(0, 5)) {
            case 0:
                candidate.heightWeight = std::max(candidate.heightWeight + quantity, 0.);
                break;
            case 1:
                candidate.scoreWeight = std::max(candidate.scoreWeight + quantity, 0.);
                break;
            case 2:
                candidate.holesWeight = std::max(candidate.holesWeight + quantity, 0.);
                break;
            case 3:
                candidate.bumpinessWeight = std::max(candidate.bumpinessWeight + quantity, 0.);
                break;
            case 4:
                candidate.emptyLinesWeight = std::max(candidate.emptyLinesWeight + quantity, 0.);
                break;
        }
    }

    static void
    deleteNLastReplacement(std::vector<Candidate> &candidates, const std::vector<Candidate> &newCandidates) {
        for (int i = 0; i < newCandidates.size(); i++) candidates.pop_back();
        for (int i = 0; i < newCandidates.size(); i++) {
            candidates.push_back(newCandidates[i]);
        }
        std::sort(candidates.begin(), candidates.end(), [&](Candidate a, Candidate b) {
            return a.fitness > b.fitness;
        });
    }

    /*
        Population size = 100
        Rounds per candidate = 5
        Max moves per round = 200
        Theoretical fitness limit = 5 * 200 * 4 / 10 = 400
    */
    void run() {
        //TODO:: add concurrency
        std::vector<Candidate> candidates;
        // Initial population generation
        for (int i = 0; i < 100; i++) {
            candidates.emplace_back(generateRandomCandidate());
        }
        std::cout << "Computing fitnesses of initial candidates.\n";
        computeFitnesses(candidates, 10, 300);
        std::sort(candidates.begin(), candidates.end(), [&](Candidate a, Candidate b) {
            return a.fitness > b.fitness;
        });

        int count = 0;
        for (int _ = 0; _ < 100; _++) {
            std::vector<Candidate> newCandidates;
            for (int i = 0; i < 30; i++) { // 30% of population
                auto pair = tournamentSelectPair(candidates, 10); // 10% of population
                Candidate candidate = crossOver(pair.first, pair.second);
                if (randomInteger(1, 100) <= 5) {
                    mutate(candidate);
//                    output(candidate);
                }
                normalize(candidate);
                newCandidates.push_back(candidate);
            }
            std::cout << "Computing fitnesses of new candidates. (" + std::to_string(count) + ")\n";
            computeFitnesses(newCandidates, 10, 300);
            std::cout
                    << "Replacing the least 30% of the population with the new candidates. (" + std::to_string(count) +
                       ")\n";
            deleteNLastReplacement(candidates, newCandidates);
            int totalFitness = 0;
            for (int i = 0; i < candidates.size(); i++) {
                totalFitness += candidates[i].fitness;
            }
            std::cout << "Average fitness = " << 1.0 * totalFitness / candidates.size() << '\n';
            std::cout << "Highest fitness = " << candidates[0].fitness << "(" << count << ")\n";
            std::cout << "Fittest candidate = " << "heightWeight:" << candidates[0].heightWeight << ',' <<
                      "bumpinessWeight:" << candidates[0].bumpinessWeight << ',' << "holesWeight:"
                      << candidates[0].holesWeight << ',' <<
                      "scoreWeight:" << candidates[0].scoreWeight <<
                      ",emptyLinesWeight:" << candidates[0].emptyLinesWeight << "(" << count << ")\n";
            count++;
        }
    };
public:
    std::mt19937 gen;

    int randomInteger(int min, int max) {
        return std::uniform_int_distribution<int>(min, max)(gen);
    }

    double randomDouble(double min, double max) {
        return std::uniform_real_distribution<double>(min, max)(gen);
    }
};