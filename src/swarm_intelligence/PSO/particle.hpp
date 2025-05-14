#pragma once

#include "util.hpp"

#include <swarm_intelligence/common/CVRP_problem.hpp>

#include <vector>


namespace SI::PSO {
    class TParticle {

    public:
        TParticle(const CVRP::TProblem& problem);
        TParticle(const CVRP::TProblem& problem, const TCoordinates& position);

        TParticle(const TParticle& other) = default;
        TParticle& operator=(const TParticle& other) = default;
        TParticle& operator=(TParticle&& other) = default;

        TDistance TotalDistance() const;
        const CVRP::TRoutes& GetRoutes() const;

        TDistance Evaluate();
        TDistance BestDistance() const;
        const TCoordinates& BestPosition() const;
        const CVRP::TRoutes& BestRoutes() const;

        void MakeTwoOpt();

        void Update(const TParameters& parameters, const TCoordinates& bestGlobalPosition, bool isDebugPrint = false);
        void DebugPrint(std::ostream& stream);

        static TDistance MakeTwoOpt(const CVRP::TProblem& problem, CVRP::TRoutes& routes, TDistance oldBestDistance);

    private:
        void InsertSegment(const TCoordinates& source, size_t start, size_t end);
        void Repair();

        void UpdateBestPosition();

    private:
        const CVRP::TProblem& Problem_;

        CVRP::TRoutes Routes_; // Routes for multiple vehicles
        TCoordinates Position_;
        TDistance TotalDistance_;

        CVRP::TRoutes BestRoutes_;
        TCoordinates BestPosition_;
        TDistance BestDistance_; // Best total distance for this particle
        int64_t IterationsWithoutImprovement_;
    };
};