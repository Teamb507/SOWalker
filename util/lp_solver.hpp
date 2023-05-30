#include <iostream>
#include <numeric>
#include <vector>
#include <unordered_map>

#include <ortools/linear_solver/linear_expr.h>
#include <ortools/linear_solver/linear_solver.h>

struct Edge_t { 
    size_t src, dst;
    double w;
};

// struct DataModel {
//     const std::vector<Edge_t>& edges;
//     const std::vector<size_t>& verts;
//     const size_t num_vert;
//     const size_t num_edge;
//     const size_t num_cache;

//     DataModel(const std::vector<Edge_t> &v_edges, const std::vector<size_t> &v_verts, size_t n_vert, size_t n_edge, size_t n_cache) : edges(v_edges), verts(v_verts), num_vert(n_vert), num_edge(n_edge), num_cache(n_cache) {}
//     DataModel(const std::vector<Edge_t> &&v_edges, const std::vector<size_t> &&v_verts, size_t n_vert, size_t n_edge, size_t n_cache) = delete;
// };

struct DataModel {
    const std::vector<Edge_t>& edges;
    const std::vector<bool>& cache_blocks;
    const size_t num_vert;
    const size_t num_edge;
    const size_t num_cache;
    const size_t num_select;

    DataModel(const std::vector<Edge_t> &v_edges, const std::vector<bool> &v_blocks, size_t n_vert, size_t n_edge, size_t n_cache, size_t n_select) : edges(v_edges), cache_blocks(v_blocks), num_vert(n_vert), num_edge(n_edge), num_cache(n_cache), num_select(n_select){}
    DataModel(const std::vector<Edge_t> &&v_edges, const std::vector<bool> &&v_blocks, size_t n_vert, size_t n_edge, size_t n_cache, size_t n_select) = delete;
};

namespace operations_research
{
    bool lp_solve_schedule(const DataModel& data, std::vector<bool>& ans_blocks)
    {
        // Create the mip solver with the SCIP backend.
        std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
        if (!solver)
        {
            LOG(WARNING) << "SCIP solver unavailable.";
            return false;
        }

        std::vector<const MPVariable *> v_e(data.num_edge);
        for (size_t i = 0; i < data.num_edge; i++)
        {
            v_e[i] = solver->MakeIntVar(0.0, 1.0, "v_e_" + std::to_string(i));
        }

        std::vector<const MPVariable *> v_v(data.num_vert);
        for (size_t i = 0; i < data.num_vert; i++)
        {
            v_v[i] = solver->MakeIntVar(0.0, 1.0, "v_v_" + std::to_string(i));
        }

        LinearExpr sum, sum_select;
        for (size_t i = 0; i < data.num_vert; i++)
        {
            sum += v_v[i];
            if(data.cache_blocks[i]) {
                sum_select += v_v[i];
            }
        }
        solver->MakeRowConstraint(sum_select == data.num_select);
        solver->MakeRowConstraint(sum == data.num_cache);

        for (size_t i = 0; i < data.num_edge; i++)
        {
            solver->MakeRowConstraint(LinearExpr(v_e[i]) <= LinearExpr(v_v[data.edges[i].src]));
            solver->MakeRowConstraint(LinearExpr(v_e[i]) <= LinearExpr(v_v[data.edges[i].dst]));
        }

        LinearExpr object_expr;
        for (size_t i = 0; i < data.num_edge; i++)
        {
            object_expr += data.edges[i].w * LinearExpr(v_e[i]);
        }

        MPObjective *const objective = solver->MutableObjective();
        objective->MaximizeLinearExpr(object_expr);

        const MPSolver::ResultStatus result_status = solver->Solve();

        // Check that the problem has an optimal solution.
        if (result_status != MPSolver::OPTIMAL)
        {
            std::cerr << "The problem does not have an optimal solution!" << std::endl;
            return false;
        }

        std::cout << "model value: " << objective->Value() << std::endl;

        for(size_t i = 0; i < data.num_vert; i++) {
            ans_blocks[i] = (v_v[i]->solution_value() == 1);
        }
        
        return true;
    }
}