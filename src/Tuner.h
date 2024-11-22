//
// Created by amemiya7 on 2024/6/24.
//

#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include "Generator.h"
#include <toml++/toml.hpp>
#include <filesystem>
#include "Ai.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include "spdlog/fmt/ostr.h"
#include "BS_thread_pool.hpp"
#include <future>


//TODO::modify linesWeight to scoreWeight
struct Candidate {
    int32_t heightWeight;
    int32_t scoreWeight;
    int32_t holesWeight;
    int32_t bumpinessWeight;
    int32_t emptyLinesWeight;
    int32_t backToBackWeight;
    int32_t comboWeight;
    int32_t tetrisWeight;
    int32_t perfectClearWeight;
    int32_t singleWeight;
    int32_t doubleWeight;
    int32_t tripleWeight;
    int32_t highestWeight;
    int32_t movementWeight;
    int32_t halfTSpinDoubleHoleWeight;
    int32_t fullTSpinDoubleHoleWeight;
    int32_t halfTSpinTripleHoleWeight;
    int32_t fullTSpinTripleHoleWeight;
    int32_t tSpinSingleWeight;
    int32_t tSpinDoubleWeight;
    int32_t tSpinTripleWeight;
    int32_t tSpinMiniSingleWeight;
    int32_t tSpinMiniDoubleWeight;
    int64_t fitness;


};

std::ostream &operator<<(std::ostream &os, const Candidate &candidate) {
    os << "fitness:" << candidate.fitness
       << ",heightWeight:" << candidate.heightWeight
       << ",scoreWeight:" << candidate.scoreWeight
       << ",holesWeight:" << candidate.holesWeight
       << ",bumpinessWeight:" << candidate.bumpinessWeight
       << ",emptyLinesWeight:" << candidate.emptyLinesWeight
       << ",backToBackWeight:" << candidate.backToBackWeight
       << ",comboWeight:" << candidate.comboWeight
       << ",tetrisWeight:" << candidate.tetrisWeight
       << ",perfectClearWeight:" << candidate.perfectClearWeight
       << ",singleWeight:" << candidate.singleWeight
       << ",doubleWeight:" << candidate.doubleWeight
       << ",tripleWeight:" << candidate.tripleWeight
       << ",highestWeight:" << candidate.highestWeight
       << ",movementWeight:" << candidate.movementWeight
       << ", halfTSpinDoubleHoleWeight:" << candidate.halfTSpinDoubleHoleWeight
       << ", fullTSpinDoubleHoleWeight:" << candidate.fullTSpinDoubleHoleWeight
       << ", candidate.halfTSpinTripleHoleWeight:" << candidate.halfTSpinTripleHoleWeight
       << ", fullTSpinTripleHoleWeight" << candidate.fullTSpinTripleHoleWeight
       << ", tSpinSingleWeight:" << candidate.tSpinSingleWeight
       << ", tSpinDoubleWeight:" << candidate.tSpinDoubleWeight
       << ", tSpinTripleWeight:" << candidate.tSpinTripleWeight
       << ", tSpinMiniSingleWeight:" << candidate.tSpinMiniSingleWeight
       << ", tSpinMiniDoubleWeight:" << candidate.tSpinMiniDoubleWeight;
    return os;
}

template<>
struct fmt::formatter<Candidate> : fmt::ostream_formatter {
};

class Tuner {
public:
    Tuner() {
        std::random_device rd;
        gen = std::mt19937(rd());
//        const std::filesystem::path projectPath(FILE_LOCATION);
        auto config = toml::parse_file("../config.toml");
        MAX_THREAD = config["tuner"]["MAX_THREAD"].value_or(1);
        if (MAX_THREAD == 0) MAX_THREAD = std::thread::hardware_concurrency();
        threadPool.reset(MAX_THREAD);
        MAX_MOVES = config["tuner"]["MAX_MOVES"].value_or(300);
        MAX_GAMES = config["tuner"]["MAX_GAMES"].value_or(10);
        MAX_POPULATIONS = config["tuner"]["MAX_POPULATIONS"].value_or(100);
        MAX_GENERATIONS = config["tuner"]["MAX_GENERATIONS"].value_or(1000);
        DATA_DIR = config["tuner"]["DATA_DIR"].value_or("");
        logFile->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [Thread:%t] [Line:%#] %v");
        aiProcessLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [Thread:%t] %v");
        spdlog::flush_every(std::chrono::seconds(5));
    }

    static void normalize(Candidate &candidate) {
        candidate.heightWeight = std::max(1, std::min(1000, candidate.heightWeight));
        candidate.scoreWeight = std::max(1, std::min(1000, candidate.scoreWeight));
        candidate.holesWeight = std::max(1, std::min(1000, candidate.holesWeight));
        candidate.bumpinessWeight = std::max(1, std::min(1000, candidate.bumpinessWeight));
        candidate.emptyLinesWeight = std::max(1, std::min(1000, candidate.emptyLinesWeight));
        candidate.backToBackWeight = std::max(1, std::min(1000, candidate.backToBackWeight));
        candidate.comboWeight = std::max(1, std::min(1000, candidate.comboWeight));
        candidate.tetrisWeight = std::max(1, std::min(1000, candidate.tetrisWeight));
        candidate.perfectClearWeight = std::max(1, std::min(1000, candidate.perfectClearWeight));
        candidate.singleWeight = std::max(1, std::min(1000, candidate.singleWeight));
        candidate.doubleWeight = std::max(1, std::min(1000, candidate.doubleWeight));
        candidate.tripleWeight = std::max(1, std::min(1000, candidate.tripleWeight));
        candidate.highestWeight = std::max(1, std::min(1000, candidate.highestWeight));
        candidate.movementWeight = std::max(1, std::min(1000, candidate.movementWeight));
        candidate.halfTSpinDoubleHoleWeight = std::max(1, std::min(1000, candidate.halfTSpinDoubleHoleWeight));
        candidate.fullTSpinDoubleHoleWeight = std::max(1, std::min(1000, candidate.fullTSpinDoubleHoleWeight));
        candidate.halfTSpinTripleHoleWeight = std::max(1, std::min(1000, candidate.halfTSpinTripleHoleWeight));
        candidate.fullTSpinTripleHoleWeight = std::max(1, std::min(1000, candidate.fullTSpinTripleHoleWeight));
        candidate.tSpinMiniDoubleWeight = std::max(1, std::min(1000, candidate.tSpinMiniDoubleWeight));
        candidate.tSpinMiniSingleWeight = std::max(1, std::min(1000, candidate.tSpinMiniSingleWeight));
        candidate.tSpinTripleWeight = std::max(1, std::min(1000, candidate.tSpinTripleWeight));
        candidate.tSpinDoubleWeight = std::max(1, std::min(1000, candidate.tSpinDoubleWeight));
        candidate.tSpinSingleWeight = std::max(1, std::min(1000, candidate.tSpinSingleWeight));
    }

    Candidate generateRandomCandidate() {
        Candidate candidate = {
                .heightWeight = randomInteger(1, 1000),
                .scoreWeight = randomInteger(1, 1000),
                .holesWeight = randomInteger(1, 1000),
                .bumpinessWeight = randomInteger(1, 1000),
                .emptyLinesWeight =randomInteger(1, 1000),
                .backToBackWeight = randomInteger(1, 1000),
                .comboWeight = randomInteger(1, 1000),
                .tetrisWeight = randomInteger(1, 1000),
                .perfectClearWeight = randomInteger(1, 1000),
                .singleWeight = randomInteger(1, 1000),
                .doubleWeight = randomInteger(1, 1000),
                .tripleWeight = randomInteger(1, 1000),
                .highestWeight = randomInteger(1, 1000),
                .movementWeight = randomInteger(1, 1000),
                .halfTSpinDoubleHoleWeight = randomInteger(1, 1000),
                .fullTSpinDoubleHoleWeight = randomInteger(1, 1000),
                .halfTSpinTripleHoleWeight = randomInteger(1, 1000),
                .fullTSpinTripleHoleWeight = randomInteger(1, 1000),
                .tSpinSingleWeight = randomInteger(1, 1000),
                .tSpinDoubleWeight = randomInteger(1, 1000),
                .tSpinTripleWeight = randomInteger(1, 1000),
                .tSpinMiniSingleWeight = randomInteger(1, 1000),
                .tSpinMiniDoubleWeight = randomInteger(1, 1000),
                .fitness = 0
        };
//        normalize(candidate);
        return candidate;
    }

    //TODO::add log file
    void computeFitnesses(std::vector<Candidate> &candidates, int numberOfGames, int maxNumberOfMoves) {
        if (MAX_THREAD == 1) {
            for (int i = 0; i < candidates.size(); i++) {
                threadRun(candidates, numberOfGames, maxNumberOfMoves, i);
            }
        } else {
            uint32_t totalThreads = MAX_THREAD;
            for (int i = 0;i < candidates.size();i ++) {
                threadPool.detach_task([&candidates, numberOfGames, maxNumberOfMoves, i, this]{
                    this->threadRun(candidates, numberOfGames, maxNumberOfMoves, i);
                });
            }
            threadPool.wait();
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

    //TODO:: change crossOver
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
                .heightWeight = static_cast<int32_t>(coefficient1 * candidate1.heightWeight +
                                                     coefficient2 * candidate2.heightWeight),
                .scoreWeight =  static_cast<int32_t>(coefficient1 * candidate1.scoreWeight +
                                                     coefficient2 * candidate2.scoreWeight),
                .holesWeight = static_cast<int32_t>(coefficient1 * candidate1.holesWeight +
                                                    coefficient2 * candidate2.holesWeight),
                .bumpinessWeight = static_cast<int32_t>(coefficient1 * candidate1.bumpinessWeight +
                                                        coefficient2 * candidate2.bumpinessWeight),
                .emptyLinesWeight = static_cast<int32_t>(coefficient1 * candidate1.emptyLinesWeight +
                                                         coefficient2 * candidate2.emptyLinesWeight),
                .backToBackWeight = static_cast<int32_t>(coefficient1 * candidate1.backToBackWeight +
                                                         coefficient2 * candidate2.backToBackWeight),
                .comboWeight = static_cast<int32_t>(coefficient1 * candidate1.comboWeight +
                                                    coefficient2 * candidate2.comboWeight),
                .tetrisWeight = static_cast<int32_t>(coefficient1 * candidate1.tetrisWeight +
                                                     coefficient2 * candidate2.tetrisWeight),
                .perfectClearWeight = static_cast<int32_t>(coefficient1 * candidate1.perfectClearWeight +
                                                           coefficient2 * candidate2.perfectClearWeight),
                .singleWeight = static_cast<int32_t>(coefficient1 * candidate1.singleWeight +
                                                     coefficient2 * candidate2.singleWeight),
                .doubleWeight = static_cast<int32_t>(coefficient1 * candidate1.doubleWeight +
                                                     coefficient2 * candidate2.doubleWeight),
                .tripleWeight = static_cast<int32_t>(coefficient1 * candidate1.tripleWeight +
                                                     coefficient2 * candidate2.tripleWeight),
                .highestWeight = static_cast<int32_t>(coefficient1 * candidate1.highestWeight +
                                                      coefficient2 * candidate2.highestWeight),
                .movementWeight = static_cast<int32_t>(coefficient1 * candidate1.movementWeight +
                                                       coefficient2 * candidate2.movementWeight),
                .halfTSpinDoubleHoleWeight = static_cast<int32_t>(coefficient1 * candidate1.halfTSpinDoubleHoleWeight +
                                                                  coefficient2 * candidate2.halfTSpinDoubleHoleWeight),
                .fullTSpinDoubleHoleWeight = static_cast<int32_t>(coefficient1 * candidate1.fullTSpinDoubleHoleWeight +
                                                                  coefficient2 * candidate2.fullTSpinDoubleHoleWeight),
                .halfTSpinTripleHoleWeight = static_cast<int32_t>(coefficient1 * candidate1.halfTSpinTripleHoleWeight +
                                                                  coefficient2 * candidate2.halfTSpinTripleHoleWeight),
                .fullTSpinTripleHoleWeight = static_cast<int32_t>(coefficient1 * candidate1.fullTSpinTripleHoleWeight +
                                                                  coefficient2 * candidate2.fullTSpinTripleHoleWeight),
                .tSpinSingleWeight = static_cast<int32_t>(coefficient1 * candidate1.tSpinSingleWeight +
                                                          coefficient2 * candidate2.tSpinSingleWeight),
                .tSpinDoubleWeight = static_cast<int32_t>(coefficient1 * candidate1.tSpinDoubleWeight +
                                                          coefficient2 * candidate2.tSpinDoubleWeight),
                .tSpinTripleWeight = static_cast<int32_t>(coefficient1 * candidate1.tSpinTripleWeight +
                                                          coefficient2 * candidate2.tSpinTripleWeight),
                .tSpinMiniSingleWeight = static_cast<int32_t>(coefficient1 * candidate1.tSpinMiniSingleWeight +
                                                              coefficient2 * candidate2.tSpinMiniSingleWeight),
                .tSpinMiniDoubleWeight = static_cast<int32_t>(coefficient1 * candidate1.tSpinMiniDoubleWeight +
                                                              coefficient2 * candidate2.tSpinMiniDoubleWeight),


        };
        normalize(candidate);
        return candidate;
    }

    void mutate(Candidate &candidate) {
        auto quantity = 0;
        for (int i = 0; i < 4; i++) {
            switch (randomInteger(0, 22)) {
                case 0:
                    quantity = randomInteger(-candidate.heightWeight + 1, candidate.heightWeight);
                    candidate.heightWeight = std::max(candidate.heightWeight + quantity, 0);
                    break;
                case 1:
                    quantity = randomInteger(-candidate.scoreWeight + 1, candidate.scoreWeight);
                    candidate.scoreWeight = std::max(candidate.scoreWeight + quantity, 0);
                    break;
                case 2:
                    quantity = randomInteger(-candidate.holesWeight + 1, candidate.holesWeight);
                    candidate.holesWeight = std::max(candidate.holesWeight + quantity, 0);
                    break;
                case 3:
                    quantity = randomInteger(-candidate.bumpinessWeight + 1, candidate.bumpinessWeight);
                    candidate.bumpinessWeight = std::max(candidate.bumpinessWeight + quantity, 0);
                    break;
                case 4:
                    quantity = randomInteger(-candidate.emptyLinesWeight + 1, candidate.emptyLinesWeight);
                    candidate.emptyLinesWeight = std::max(candidate.emptyLinesWeight + quantity, 0);
                    break;
                case 5:
                    quantity = randomInteger(-candidate.backToBackWeight + 1, candidate.backToBackWeight);
                    candidate.backToBackWeight = std::max(candidate.backToBackWeight + quantity, 0);
                    break;
                case 6:
                    quantity = randomInteger(-candidate.comboWeight + 1, candidate.comboWeight);
                    candidate.comboWeight = std::max(candidate.comboWeight + quantity, 0);
                    break;
                case 7:
                    quantity = randomInteger(-candidate.tetrisWeight + 1, candidate.tetrisWeight);
                    candidate.tetrisWeight = std::max(candidate.tetrisWeight + quantity, 0);
                    break;
                case 8:
                    quantity = randomInteger(-candidate.perfectClearWeight + 1, candidate.perfectClearWeight);
                    candidate.perfectClearWeight = std::max(candidate.perfectClearWeight + quantity, 0);
                    break;
                case 9:
                    quantity = randomInteger(-candidate.tSpinSingleWeight + 1, candidate.tSpinSingleWeight);
                    candidate.tSpinSingleWeight = std::max(candidate.tSpinSingleWeight + quantity, 0);
                    break;
                case 10:
                    quantity = randomInteger(-candidate.singleWeight + 1, candidate.singleWeight);
                    candidate.singleWeight = std::max(candidate.singleWeight + quantity, 0);
                    break;
                case 11:
                    quantity = randomInteger(-candidate.doubleWeight + 1, candidate.doubleWeight);
                    candidate.doubleWeight = std::max(candidate.doubleWeight + quantity, 0);
                    break;
                case 12:
                    quantity = randomInteger(-candidate.tripleWeight + 1, candidate.tripleWeight);
                    candidate.tripleWeight = std::max(candidate.tripleWeight + quantity, 0);
                    break;
                case 13:
                    quantity = randomInteger(-candidate.highestWeight + 1, candidate.highestWeight);
                    candidate.highestWeight = std::max(candidate.highestWeight + quantity, 0);
                    break;
                case 14:
                    quantity = randomInteger(-candidate.movementWeight + 1, candidate.movementWeight);
                    candidate.movementWeight = std::max(candidate.movementWeight + quantity, 0);
                    break;
                case 15:
                    quantity = randomInteger(-candidate.halfTSpinDoubleHoleWeight + 1,
                                             candidate.halfTSpinDoubleHoleWeight);
                    candidate.halfTSpinDoubleHoleWeight = std::max(candidate.halfTSpinDoubleHoleWeight + quantity, 0);
                case 16:
                    quantity = randomInteger(-candidate.fullTSpinDoubleHoleWeight + 1,
                                             candidate.fullTSpinDoubleHoleWeight);
                    candidate.fullTSpinDoubleHoleWeight = std::max(candidate.fullTSpinDoubleHoleWeight + quantity, 0);
                case 17:
                    quantity = randomInteger(-candidate.halfTSpinTripleHoleWeight + 1,
                                             candidate.halfTSpinTripleHoleWeight);
                    candidate.halfTSpinTripleHoleWeight = std::max(candidate.halfTSpinTripleHoleWeight + quantity, 0);
                case 18:
                    quantity = randomInteger(-candidate.fullTSpinTripleHoleWeight + 1,
                                             candidate.fullTSpinTripleHoleWeight);
                    candidate.fullTSpinTripleHoleWeight = std::max(candidate.fullTSpinTripleHoleWeight + quantity, 0);
                case 19:
                    quantity = randomInteger(-candidate.tSpinDoubleWeight + 1, candidate.tSpinDoubleWeight);
                    candidate.tSpinDoubleWeight = std::max(candidate.tSpinDoubleWeight + quantity, 0);
                    break;
                case 20:
                    quantity = randomInteger(-candidate.tSpinTripleWeight + 1, candidate.tSpinTripleWeight);
                    candidate.tSpinTripleWeight = std::max(candidate.tSpinTripleWeight + quantity, 0);
                    break;
                case 21:
                    quantity = randomInteger(-candidate.tSpinMiniSingleWeight + 1, candidate.tSpinMiniSingleWeight);
                    candidate.tSpinMiniSingleWeight = std::max(candidate.tSpinMiniSingleWeight + quantity, 0);
                    break;
                case 22:
                    quantity = randomInteger(-candidate.tSpinMiniDoubleWeight + 1, candidate.tSpinMiniDoubleWeight);
                    candidate.tSpinMiniDoubleWeight = std::max(candidate.tSpinMiniDoubleWeight + quantity, 0);
                    break;
            }
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
    void serialize(const std::vector<Candidate> &candidates, int generation) {
        auto table = toml::table{
                {"Candidates", toml::table{}}
        };
        table["Candidates"].as_table()->emplace("size", (int) candidates.size());
        for (int i = 0; i < candidates.size(); i++) {
            auto candidateArray = toml::array{
                    candidates[i].heightWeight,
                    candidates[i].scoreWeight,
                    candidates[i].holesWeight,
                    candidates[i].bumpinessWeight,
                    candidates[i].emptyLinesWeight,
                    candidates[i].backToBackWeight,
                    candidates[i].comboWeight,
                    candidates[i].tetrisWeight,
                    candidates[i].perfectClearWeight,
                    candidates[i].singleWeight,
                    candidates[i].doubleWeight,
                    candidates[i].tripleWeight,
                    candidates[i].highestWeight,
                    candidates[i].movementWeight,
                    candidates[i].halfTSpinDoubleHoleWeight,
                    candidates[i].fullTSpinDoubleHoleWeight,
                    candidates[i].halfTSpinTripleHoleWeight,
                    candidates[i].fullTSpinTripleHoleWeight,
                    candidates[i].tSpinSingleWeight,
                    candidates[i].tSpinDoubleWeight,
                    candidates[i].tSpinTripleWeight,
                    candidates[i].tSpinMiniSingleWeight,
                    candidates[i].tSpinMiniDoubleWeight,
                    candidates[i].fitness,
            };
            table["Candidates"].as_table()->emplace(fmt::format("Candidate{}", i), candidateArray);
        }
        std::ofstream file(fmt::format("../logs/AiTrainData_Generation{}.toml", generation));
        file << table;
        file.close();
    }

    void run() {
        auto logger = spdlog::basic_logger_mt("AI_DATA", "../logs/AiTrainData.txt");
        //TODO:: add concurrency
        std::vector<Candidate> candidates(MAX_POPULATIONS);
        // Initial population generation
        if (DATA_DIR.empty()) {
            for (int i = 0; i < MAX_POPULATIONS; i++) {
                candidates[i] = generateRandomCandidate();
            }
            logger->info("Computing fitnesses of initial candidates.");
            //timer
            computeFitnesses(candidates, MAX_GAMES, MAX_MOVES);
            std::sort(candidates.begin(), candidates.end(), [&](Candidate a, Candidate b) {
                return a.fitness > b.fitness;
            });
            serialize(candidates, 0);
        } else {
            logger->info("Load data from {}", DATA_DIR);
            auto table = toml::parse_file(DATA_DIR);
            auto candidateTable = table["Candidates"].as_table();
            for (int i = 0;i < MAX_POPULATIONS;i ++) {
                auto candidateArray = candidateTable->at(fmt::format("Candidate{}", i)).as_array();
                candidates[i] = {
                        .heightWeight = candidateArray[0].value_or(0),
                        .scoreWeight = candidateArray[1].value_or(0),
                        .holesWeight = candidateArray[2].value_or(0),
                        .bumpinessWeight = candidateArray[3].value_or(0),
                        .emptyLinesWeight = candidateArray[4].value_or(0),
                        .backToBackWeight = candidateArray[5].value_or(0),
                        .comboWeight = candidateArray[6].value_or(0),
                        .tetrisWeight = candidateArray[7].value_or(0),
                        .perfectClearWeight = candidateArray[8].value_or(0),
                        .singleWeight = candidateArray[9].value_or(0),
                        .doubleWeight = candidateArray[10].value_or(0),
                        .tripleWeight = candidateArray[11].value_or(0),
                        .highestWeight = candidateArray[12].value_or(0),
                        .movementWeight = candidateArray[13].value_or(0),
                        .halfTSpinDoubleHoleWeight = candidateArray[14].value_or(0),
                        .fullTSpinDoubleHoleWeight = candidateArray[15].value_or(0),
                        .halfTSpinTripleHoleWeight = candidateArray[16].value_or(0),
                        .fullTSpinTripleHoleWeight = candidateArray[17].value_or(0),
                        .tSpinSingleWeight = candidateArray[18].value_or(0),
                        .tSpinDoubleWeight = candidateArray[19].value_or(0),
                        .tSpinTripleWeight = candidateArray[20].value_or(0),
                        .tSpinMiniSingleWeight = candidateArray[21].value_or(0),
                        .tSpinMiniDoubleWeight = candidateArray[22].value_or(0),
                        .fitness = candidateArray[23].value_or(0)
                };
            }
        }

        int count = 0;
        for (int generation = 1; generation <= MAX_GENERATIONS; generation++) {
            std::vector<Candidate> newCandidates;
            for (int i = 0; i < 30; i++) { // 30% of population
                auto pair = tournamentSelectPair(candidates, 10); // 10% of population
                Candidate candidate = crossOver(pair.first, pair.second);
                if (randomInteger(1, 100) <= 10) {
                    mutate(candidate);
//                    output(candidate);
                }
                normalize(candidate);
                newCandidates.push_back(candidate);
            }
            logger->info("Computing fitnesses of new candidates. ({}).", count);
            computeFitnesses(newCandidates, MAX_GAMES, MAX_MOVES);
            logger->info("Replacing the least 30% of the population with the new candidates. ({}).", count);
            deleteNLastReplacement(candidates, newCandidates);
            int totalFitness = 0;
            for (int i = 0; i < candidates.size(); i++) {
                totalFitness += candidates[i].fitness;
            }
            logger->info("Average fitness = {}, Highest fitness = {} ({})\nFittest candidate = {}, ",
                         1.0 * totalFitness / candidates.size(), candidates[0].fitness, count, candidates[0]);
            serialize(candidates, generation);
            count++;
        }
    };

private:
    BS::thread_pool threadPool;
    std::mt19937 gen;
    uint32_t MAX_THREAD = 1;
    uint32_t MAX_MOVES = 300;
    uint32_t MAX_GAMES = 10;
    uint32_t MAX_POPULATIONS = 100;
    uint32_t MAX_GENERATIONS = 1000;
    std::string_view DATA_DIR = "";
    std::shared_ptr<spdlog::logger> logFile = spdlog::basic_logger_mt<spdlog::async_factory>("Tuner",
                                                                                             "../logs/Tuner.txt");
    std::shared_ptr<spdlog::logger> aiProcessLogger = spdlog::basic_logger_mt<spdlog::async_factory>("AI_PROCESS",
                                                                                                     "../logs/AiProcess.txt");
//    std::mutex mu;
//    int taskCount = 0;

    void threadRun(std::vector<Candidate> &candidates, int numberOfGames, int maxNumberOfMoves, int index) {
        Candidate &candidate = candidates[index];
        AI ai = AI(candidate.heightWeight, candidate.scoreWeight, candidate.holesWeight,
                   candidate.bumpinessWeight, candidate.emptyLinesWeight, candidate.backToBackWeight,
                   candidate.comboWeight, candidate.tetrisWeight, candidate.perfectClearWeight,
                   candidate.singleWeight, candidate.doubleWeight, candidate.tripleWeight,
                   candidate.highestWeight, candidate.movementWeight, candidate.halfTSpinDoubleHoleWeight,
                   candidate.fullTSpinDoubleHoleWeight, candidate.halfTSpinTripleHoleWeight,
                   candidate.fullTSpinTripleHoleWeight, candidate.tSpinSingleWeight, candidate.tSpinDoubleWeight,
                      candidate.tSpinTripleWeight, candidate.tSpinMiniSingleWeight, candidate.tSpinMiniDoubleWeight);
        uint64_t totalScore = 0;
        int totalMovement = 0;
        for (int j = 0; j < numberOfGames; j++) {
            Grid grid(10, 22);
            Generator rpg{};

            uint64_t score = 0;
            int numberOfMoves = 0;
            bool isHoldBlock = false;
            Block holdBlock{};
//            aiProcessLogger->info("Game Started. TaskID:{} Game:{};", index, j);
            while ((numberOfMoves++) < maxNumberOfMoves && !grid.exceed()) {
                Block workingPiece{};
                if (isHoldBlock) {
                    workingPiece = holdBlock;
                    holdBlock = Block{};
                    isHoldBlock = false;
                    numberOfMoves--;
                } else {
                    workingPiece = rpg.nextBlock();
                }
                std::vector<Block> workingPieces = {workingPiece};
                for (const auto &piece: rpg.seeNextBlocks(9)) workingPieces.push_back(piece);
                auto [_nouse, movements, _grid_nouse_1, _bestBlock] = ai.best_(grid, workingPieces);
                //TODO::move
                for (auto movement: movements) {
                    if (movement == Movement::Hold) {
                        if (movements.size() != 1) {
                            logFile->debug("Hold is not the only movement");
                        }
                        grid.hold(workingPiece);
                        if (workingPiece.empty()) continue;
                        isHoldBlock = true;
                        holdBlock = workingPiece;
                        continue;
//                        auto [_score_nouse, secondMovements, _grid_nouse_2, _aisdjas] = ai.best_(grid, workingPieces);
//                        for (auto secondMovement: secondMovements) {
//                            if (secondMovement == Movement::Hold) {
//                                logFile->debug("Hold is not the only movement");
//                            }
//                            utility::move(grid, workingPiece, secondMovement);
//                        }
                    } else if (movement == Movement::softDrop) {
                        workingPiece.totalMovements += 10;
                        utility::move(grid, workingPiece, Movement::Down);
                    } else {
                        utility::move(grid, workingPiece, movement);
                    }
                }
                if (!workingPiece.empty()) {
                    utility::insertBlock(grid, workingPiece);
                    score += utility::getScore(grid, workingPiece);
                }
            }
//            aiProcessLogger->info("Game Ended. TaskID:{} Game:{} completed. Score:{}; Total Movement:{};", index, j,
//                                  score, numberOfMoves);
            totalMovement += numberOfMoves;
            totalScore += score;
        }
        aiProcessLogger->info("TaskID:{} completed.", index);
        candidate.fitness = totalScore;
    }

    int randomInteger(int min, int max) {
        return std::uniform_int_distribution<int>(min, max)(gen);
    }

};