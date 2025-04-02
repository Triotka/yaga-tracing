#include "metrics/Metrics.h"
using namespace yaga;

Metrics& Metrics::instance()
{
    static Metrics instance; // Only one instance is created
    return instance;
}

Metrics::Metrics()
{
    log_file.open("metrics.csv", std::ios::app);
    if (!log_file.is_open())
    {
        throw std::runtime_error("Failed to open log file.");
    }
}

Metrics::~Metrics() { log_file.close(); }

void Metrics::close_log_file() { log_file.close(); }

// SAT core

void Metrics::log_core_before_propagation(int decision_level, int trail_size, int total_decisions,
                                          int total_conflicts)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,before_propagation," << decision_level << "," << trail_size << ","
             << total_decisions << "," << total_conflicts << "\n";
}

void Metrics::log_core_after_propagation(int decision_level, int trail_size,
                                         std::vector<Clause> const& conflicts)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,after_propagation," << decision_level << "," << trail_size << ","
             << conflicts.size() << "\n";
}

void Metrics::log_core_decision(Variable const& var, int decision_level, int trail_size,
                                int total_decisions, int total_backtracks)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,decision," << var.ord() << "," << decision_level << "," << trail_size << ","
             << total_decisions << "," << total_backtracks << "\n";
}

void Metrics::log_core_learned_clause(Clause const& clause, int decision_level, int trail_size,
                                      int total_learned, bool is_semantic_split)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,learned_clause," << clause.size() << "," << decision_level << ","
             << trail_size << "," << total_learned << ","
             << (is_semantic_split ? "semantic" : "UIP") << "\n";
}

void Metrics::log_core_before_backtrack(int decision_level, int trail_size, int total_decisions)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,before_backtrack," << decision_level << "," << trail_size << ","
             << total_decisions << "\n";
}

void Metrics::log_core_after_backtrack(int decision_level, int trail_size, bool is_semantic_split)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,after_backtrack," << decision_level << "," << trail_size << ","
             << (is_semantic_split ? "semantic" : "UIP") << "\n";
}

void Metrics::log_core_before_restart(int restart_count, int decision_level, int trail_size,
                                      int total_conflicts)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,before_restart," << restart_count << "," << decision_level << ","
             << trail_size << "," << total_conflicts << "\n";
}

void Metrics::log_core_conflict_analysis_start(int trail_size, int decision_level,
                                               std::vector<Clause> const& conflict_clauses)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,conflict_analysis_start," << trail_size << "," << decision_level << ","
             << conflict_clauses.size() << "\n";
}

void Metrics::log_core_conflict_analysis_end(int learned_count, int backtrack_level,
                                             int decision_level)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,conflict_analysis_end," << learned_count << "," << backtrack_level << ","
             << decision_level << "\n";
}

void Metrics::log_core_search_end(int decision_level, int trail_size, int total_conflicts,
                                  int total_conflict_clauses, int total_learned_clauses,
                                  int total_decisions, int total_backtracks)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "core,search_end," << decision_level << "," << trail_size << "," << total_conflicts
             << "," << total_conflict_clauses << "," << total_learned_clauses << ","
             << total_decisions << "," << total_backtracks << "\n";
}

// Boolean theory

void Metrics::log_bool_literal_propagation(int decision_level, int trail_size, Literal lit,
                                           Clause const* reason)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    // Format the propagated literal
    log_file << "bool,literal_propagation,";
    log_file << (lit.is_negation() ? "¬" : "") << lit.var().ord() << ",";

    // Format the reason clause as a space‐separated list
    if (reason)
    {
        bool first = true;
        for (auto& oth : *reason)
        {
            if (!first)
                log_file << " ";
            log_file << (oth.is_negation() ? "¬" : "") << oth.var().ord();
            first = false;
        }
    }
    else
    {
        log_file << "null";
    }

    // Now the numeric fields
    log_file << "," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_bool_propagation_success(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "bool,propagation_success," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_bool_propagation_failure(int decision_level, int trail_size,
                                           std::optional<Clause> conflict)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "bool,propagation_failure," << decision_level << "," << trail_size << ",";

    if (conflict.has_value())
    {
        // Print the conflict clause
        bool first = true;
        for (auto& lit : conflict.value())
        {
            if (!first)
                log_file << " ";
            log_file << (lit.is_negation() ? "¬" : "") << lit.var().ord();
            first = false;
        }
    }
    else
    {
        log_file << "N/A";
    }
    log_file << "\n";
}

void Metrics::log_bool_watchlist_size(int var_ord, int watchlist_size, int decision_level,
                                      int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "bool,watchlist_size," << var_ord << "," << watchlist_size << "," << decision_level
             << "," << trail_size << "\n";
}

void Metrics::log_bool_watchlist_update(int var_ord, Clause const& clause, int new_index)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;

    log_file << "bool,watchlist_update," << var_ord << "," << new_index << ",";

    // Print the clause whose watch moved
    bool first = true;
    for (auto& lit : clause)
    {
        if (!first)
            log_file << " ";
        log_file << (lit.is_negation() ? "¬" : "") << lit.var().ord();
        first = false;
    }
    log_file << "\n";
}

// Uninterpreted functions
void Metrics::log_func_propagation_start(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "func,prop_start," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_func_assignment(int var_ord, int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "func,assignment," << var_ord << "," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_func_watch_encountered(yaga::terms::term_t term, int var_ord, int decision_level,
                                         int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "func,watch_trigger," << term.x << "," << var_ord << "," << decision_level << ","
             << trail_size << "\n";
}

void Metrics::log_func_watch_fully_assigned(yaga::terms::term_t term, int decision_level,
                                            int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "func,watch_full," << term.x << "," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_func_conflict_encountered(yaga::terms::term_t term, int num_clauses,
                                            int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "func,conflict," << term.x << "," << num_clauses << "," << decision_level << ","
             << trail_size << "\n";
}

void Metrics::log_func_propagation_end(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "func,prop_end," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_func_watch_registered(yaga::terms::term_t term, int arity)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "func,watch_registered," << term.x << "," << arity << "\n";
}

// LRA

void Metrics::log_lra_propagation_start(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,prop_start," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_lra_propagate_bounds_start(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,bounds_start," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_lra_propagate_bounds_end(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,bounds_end," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_lra_propagate_unassigned_start(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,unassigned_start," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_lra_propagate_unassigned_end(int decision_level, int trail_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,unassigned_end," << decision_level << "," << trail_size << "\n";
}

void Metrics::log_lra_propagation_end(int decision_level, int trail_size,
                                      std::vector<Variable> const& variables,
                                      std::vector<Clause> const& conflicts)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    // count how many vars and how many conflict clauses
    log_file << "lra,prop_end," << decision_level << "," << trail_size << "," << variables.size()
             << "," << conflicts.size() << "\n";
}

void Metrics::log_lra_replace_watch_success(Linear_constraint<Rational> const& cons, int old_watch,
                                            int new_watch, int watch_index)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,replace_watch_ok," << cons << "," << old_watch << "," << new_watch << ","
             << watch_index << "\n";
    yaga::operator<<(log_file, cons);
}

void Metrics::log_lra_replace_watch_fail(Linear_constraint<Rational>& cons)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,replace_watch_fail," << cons << "\n";
}

void Metrics::log_lra_watchlist_size(int var_ord, int watchlist_size, int decision_level)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,watchlist_size," << var_ord << "," << watchlist_size << "," << decision_level
             << "\n";
}

void Metrics::log_lra_remove_watch(Linear_constraint<Rational>& cons, int var_ord)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,remove_watch," << cons << "," << var_ord << "\n";
}

void Metrics::log_lra_fully_assigned(Linear_constraint<Rational>& cons, int decision_level)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,fully_assigned," << cons << "," << decision_level << "\n";
}

void Metrics::log_lra_unit_on_trail(Linear_constraint<Rational>& cons, int lit_ord)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,unit_on_trail," << cons << "," << lit_ord << "\n";
}

void Metrics::log_lra_propagate_conflict(Linear_constraint<Rational>& cons, int decision_level)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,prop_conflict," << cons << "," << decision_level << "\n";
}

void Metrics::log_lra_unit_propagation(Linear_constraint<Rational>& cons, int lit_ord)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,unit_propagation," << cons << "," << lit_ord << "\n";
}

void Metrics::log_lra_bound_conflict(int var_ord, int conflict_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,bound_conflict," << var_ord << "," << conflict_size << "\n";
}

void Metrics::log_lra_inequality_conflict(int var_ord, int conflict_size)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,ineq_conflict," << var_ord << "," << conflict_size << "\n";
}

void Metrics::log_lra_initial_decision(Variable const& var, bool used_cache, Rational const& value,
                                       bool bounds_allowed)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,init_decision," << var.ord() << "," << used_cache << "," << value << ","
             << bounds_allowed << "\n";
}

void Metrics::log_lra_integer_decision(Variable const& var,
                                       std::optional<Rational> const& int_value)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,int_decision," << var.ord() << ",";
    if (int_value.has_value())
    {
        log_file << int_value.value() << "\n";
    }
    else
    {
        log_file << "N/A" << "\n";
    }
}

void Metrics::log_lra_bisection_decision(Variable const& var, Rational const& value, int steps,
                                         Rational const& ub, Rational const& lb)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (!log_file.is_open())
        return;
    log_file << "lra,bisect_decision," << var.ord() << "," << value << "," << steps << "," << ub
             << "," << lb << "\n";
}
