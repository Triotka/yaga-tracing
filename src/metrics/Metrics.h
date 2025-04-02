#ifndef METRICS_H
#define METRICS_H

#include "../Variable.h"
#include "Clause.h"
#include "Linear_constraints.h"
#include "Term_types.h"
#include <fstream>
#include <mutex>
#include <optional>

using namespace yaga;

class Metrics {
public:
    // Get the single instance of Metrics
    static Metrics& instance();

    // Closes the log file
    void close_log_file();
    
// SAT core

    /** Logs before propagation is done by a plugin.
     *
     * @param decision_level current decision level
     * @param trail_size current trail size
     * @param total_decisions the total number of decisions so far
     * @param total_conflicts the total number of conflicts so far
     */
    void log_core_before_propagation(int decision_level, int trail_size, int total_decisions,
                                     int total_conflicts);

    /** Logs after propagation done by a plugin.
     *
     * @param decision_level the current decision level
     * @param trail_size the current trail size
     * @param num_conflicts the number of conflicts detected during propagation
     */
    void log_core_after_propagation(int decision_level, int trail_size,
                                    std::vector<Clause> const& num_conflicts);

    /** Logs a decision made.
     *
     * @param var the variable that was decided
     * @param decision_level the decision level at which the variable was decided
     * @param trail_size the trail size at the moment of decision
     * @param total_decisions the total number of decisions so far
     * @param total_backtracks the total number of backtracks so far
     */
    void log_core_decision(Variable const& var, int decision_level, int trail_size,
                           int total_decisions, int total_backtracks);

    /** Logs a clause learned.
     *
     * @param clause the learned clause
     * @param decision_level the decision level at which the clause was learned
     * @param trail_size the size of the trail at the time of learning
     * @param total_learned the total number of learned clauses so far
     * @param is_semantic_split true if the clause came from a semantic split, false if from UIP
     * conflict
     */
    void log_core_learned_clause(Clause const& clause, int decision_level, int trail_size,
                                 int total_learned, bool is_semantic_split);

    /** Logs before backtracking.
     *
     * @param decision_level the current decision level
     * @param trail_size the current trail size
     * @param total_decisions total number of decisions made during search.
     */
    void log_core_before_backtrack(int decision_level, int trail_size, int total_decisions);

    /** Logs after a backtracking
     *
     * @param decision_level the current new decision level
     * @param trail_size the current trail size
     * @param is_semantic_split a for the backtrack (e.g., "UIP" or "split")
     */
    void log_core_after_backtrack(int decision_level, int trail_size, bool is_semantic_split);

    /** Logs before a restart
     *
     * @param restart_count the number of restarts so far
     * @param decision_level the current decision level
     * @param trail_size the current trail size
     * @param total_conflicts the total number of conflicts so far
     */
    void log_core_before_restart(int restart_count, int decision_level, int trail_size,
                                 int total_conflicts);

    /** Logs the start of conflict analysis.
     *
     * @param trail_size       The size of the trail at the moment conflicts are first seen.
     * @param decision_level   The current decision level when analysis begins.
     * @param conflict_clauses The vector of raw conflict clauses to be analyzed.
     */
    void log_core_conflict_analysis_start(int trail_size, int decision_level,
                                          std::vector<Clause> const& conflict_clauses);

    /** Logs the end of conflict analysis.
     *
     * @param learned_count    The number of learned clauses produced.
     * @param backtrack_level  The decision level to which we will backtrack.
     * @param decision_level   The current decision level after analysis.
     */
    void log_core_conflict_analysis_end(int learned_count, int backtrack_level, int decision_level);

    /** Logs at the end of the search
     *
     *
     * @param decision_level         Final decision level in the trail.
     * @param trail_size             Number of assignments in the trail at termination.
     * @param total_conflicts        Total number of conflicts encountered.
     * @param total_conflict_clauses Total number of conflict clauses analyzed.
     * @param total_learned_clauses  Total number of clauses learned.
     * @param total_decisions        Total number of decisions made during search.
     * @param total_backtracks       Total number of backtracks made during search.
     */
    void log_core_search_end(int decision_level, int trail_size, int total_conflicts,
                             int total_conflict_clauses, int total_learned_clauses,
                             int total_decisions, int total_backtracks);

    // Bool theory

    /** Logs a literal propagation.
     *
     * @param decision_level  The decision level at propagation time.
     * @param trail_size      The size of the trail at propagation time.
     * @param lit             The literal that was propagated.
     * @param reason          The clause that was the reason for propagation.
     */
    void log_bool_literal_propagation(int decision_level, int trail_size, Literal lit,
                                      Clause const* reason);

    /** Logs a successful Boolean‐theory propagation step.
     *
     * @param theory          String identifier of the theory (e.g. "Bool_theory").
     * @param decision_level  The current decision level after propagation.
     * @param trail_size      The size of the trail after propagation.
     */
    void log_bool_propagation_success(int decision_level, int trail_size);

    /** Logs a failed Boolean‐theory propagation.
     *
     * @param decision_level  The current decision level at conflict discovery.
     * @param trail_size      The size of the trail at conflict discovery.
     * @param conflict        The conflict clause produced.
     */
    void log_bool_propagation_failure(int decision_level, int trail_size,
                                      std::optional<Clause> conflict);

    /** Logs the size of a watchlist falsification.
     *
     * @param var_ord         The variable whose watchlist used.
     * @param watchlist_size  The number of clauses watching that variable.
     * @param decision_level  The current decision level.
     * @param trail_size      The current trail size.
     */
    void log_bool_watchlist_size(int var_ord, int watchlist_size, int decision_level,
                                 int trail_size);

    /** Logs an update of a Boolean watchlist (i.e. the second watch was moved).
     *
     * @param var_ord         The variable whose watch was updated.
     * @param clause          The clause whose watch pointer moved.
     * @param new_index       The new watch index within the clause.
     */
    void log_bool_watchlist_update(int var_ord, Clause const& clause, int new_index);

// Uninterpreted functions

    /** Logs the start of function‐term propagation.
     *
     * @param decision_level  The current decision level.
     * @param trail_size      The size of the trail at propagation start.
     */
    void log_func_propagation_start(int decision_level, int trail_size);

    /** Logs each assignment seen by the UF plugin.
     *
     * @param var_ord         The ordinal of the Boolean/Rational var assigned.
     * @param decision_level  Current decision level.
     * @param trail_size      Current trail size.
     */
    void log_func_assignment(int var_ord, int decision_level, int trail_size);

    /** Logs when a watchlist entry for a function application is triggered.
     *
     * @param term         The application term
     * @param var_ord         The watched variable whose assignment triggered it.
     * @param decision_level  Current decision level.
     * @param trail_size      Current trail size.
     */
    void log_func_watch_encountered(yaga::terms::term_t term, int var_ord, int decision_level,
                                    int trail_size);

    /** Logs when all arguments of an application are assigned.
     *
     * @param term        The application term
     * @param decision_level  Current decision level.
     * @param trail_size      Current trail size.
     */
    void log_func_watch_fully_assigned(yaga::terms::term_t term, int decision_level,
                                       int trail_size);

    /** Logs conflicts discovered for a function application.
     *
     * @param term         The application term.
     * @param num_clauses     Number of clauses (in the conflict vector).
     * @param decision_level  Current decision level.
     * @param trail_size      Current trail size.
     */
    void log_func_conflict_encountered(yaga::terms::term_t term, int num_clauses,
                                       int decision_level, int trail_size);

    /** Logs the end of function‐term propagation.
     *
     * @param decision_level  The decision level after propagation.
     * @param trail_size      The trail size after propagation.
     */
    void log_func_propagation_end(int decision_level, int trail_size);

    /** Logs the registration of a new application term.
     *
     * @param term  The application term.
     * @param arity    Number of arguments + 1 (for the "result" var).
     */
    void log_func_watch_registered(yaga::terms::term_t term, int arity);

// LRA
    /** Logs the start of LRA propagation.
     *
     * @param decision_level  The current decision level.
     * @param trail_size      The size of the trail when propagation begins.
     */
    void log_lra_propagation_start(int decision_level, int trail_size);

    /** Logs the start of bounds propagation.
     *
     * @param decision_level  The current decision level.
     * @param trail_size      The size of the trail when bounds propagation begins.
     */
    void log_lra_propagate_bounds_start(int decision_level, int trail_size);

    /** Logs the end of bounds propagation.
     *
     * @param decision_level  The decision level after bounds propagation.
     * @param trail_size      The trail size after bounds propagation.
     */
    void log_lra_propagate_bounds_end(int decision_level, int trail_size);

    /** Logs the start of unassigned‐variable propagation.
     *
     * @param decision_level  The current decision level.
     * @param trail_size      The size of the trail when unassigned propagation begins.
     */
    void log_lra_propagate_unassigned_start(int decision_level, int trail_size);

    /** Logs the end of unassigned‐variable propagation.
     *
     * @param decision_level  The decision level after unassigned propagation.
     * @param trail_size      The trail size after unassigned propagation.
     */
    void log_lra_propagate_unassigned_end(int decision_level, int trail_size);

    /** Logs the end of LRA propagation phase.
     *
     * @param decision_level  The decision level after propagation.
     * @param trail_size      The trail size after propagation.
     * @param variables       The list of Variables that were examined.
     * @param conflicts       Any conflict clauses produced.
     */
    void log_lra_propagation_end(int decision_level, int trail_size,
                                 std::vector<Variable> const& variables,
                                 std::vector<Clause> const& conflicts);

    /** Logs a successful watch‐replacement in a constraint.
     *
     * @param cons           The constraint.
     * @param old_watch      The variable ordinal that was replaced.
     * @param new_watch      The new watched variable ordinal.
     * @param watch_index    The index in the constraint where the watch moved.
     */
    void log_lra_replace_watch_success(Linear_constraint<Rational> const& cons, int old_watch,
                                       int new_watch, int watch_index);

    /** Logs a failure to replace a watch (constraint remains on the same two variables).
     *
     * @param cons  The constraint
     */
    void log_lra_replace_watch_fail(Linear_constraint<Rational>& cons);

    /** Logs the size of a constraint’s watchlist before scanning it.
     *
     * @param var_ord         The variable whose watchlist is being inspected.
     * @param watchlist_size  How many watched entries there are.
     * @param decision_level  Current decision level.
     */
    void log_lra_watchlist_size(int var_ord, int watchlist_size, int decision_level);

    /** Logs when a watched constraint is removed from its watchlist.
     *
     * @param cons  The constraint.
     * @param var_ord  The variable whose watchlist lost this constraint.
     */
    void log_lra_remove_watch(Linear_constraint<Rational>& cons, int var_ord);

    /** Logs when a constraint becomes fully assigned (no propagation needed).
     *
     * @param cons         The constraint.
     * @param decision_level  Current decision level.
     */
    void log_lra_fully_assigned(Linear_constraint<Rational>& cons, int decision_level);

    /** Logs when a unit constraint is re‐encountered on the trail.
     *
     * @param cons   The unit constraint.
     * @param lit_ord   The literal that’s being propagated.
     */
    void log_lra_unit_on_trail(Linear_constraint<Rational>& cons, int lit_ord);

    /** Logs when a fully‐assigned constraint is used to produce a conflict.
     *
     * @param cons         The conflicting constraint.
     * @param decision_level  Current decision level.
     */
    void log_lra_propagate_conflict(Linear_constraint<Rational>& cons, int decision_level);

    /** Logs when a unit constraint is detected but the Boolean var is not yet on the trail.
     *
     * @param cons   The unit constraint.
     * @param lit_ord   The literal (variable ordinal + polarity) to be propagated later.
     */
    void log_lra_unit_propagation(Linear_constraint<Rational>& cons, int lit_ord);

    /** Logs a conflict discovered during bound‐checking.
     *
     * @param var_ord         The ordinal of the rational variable.
     * @param conflict_size   Number of literals in the conflict clause.
     */
    void log_lra_bound_conflict(int var_ord, int conflict_size);

    /** Logs an inequality conflict discovered during bound‐checking.
     *
     * @param var_ord         The ordinal of the rational variable.
     * @param conflict_size   Number of literals in the conflict clause.
     */
    void log_lra_inequality_conflict(int var_ord, int conflict_size);

    /** Logs the initial LRA decision before any bound checks.
     *
     * @param var            The variable being decided.
     * @param used_cache     Whether a cached value was used.
     * @param value          The chosen (candidate) value.
     * @param bounds_allowed Whether that value is within the current bounds.
     */
    void log_lra_initial_decision(Variable const& var, bool used_cache, Rational const& value,
                                  bool bounds_allowed);

    /** Logs when the initial LRA decision is replaced by an integer.
     *
     * @param var        The variable being decided.
     * @param int_value  The integer value that replaced the candidate.
     */
    void log_lra_integer_decision(Variable const& var, std::optional<Rational> const& int_value);

    /** Logs the bisection search steps to find a decision value.
     *
     * @param var      The variable being decided.
     * @param value    The final value selected.
     * @param steps    Number of bisection iterations performed.
     * @param ub       The original upper bound.
     * @param lb       The original lower bound.
     */
    void log_lra_bisection_decision(Variable const& var, Rational const& value, int steps,
                                    Rational const& ub, Rational const& lb);

private:
    Metrics();
    ~Metrics();
    Metrics(Metrics const&) = delete;            // Prevent copy
    Metrics& operator=(Metrics const&) = delete; // Prevent assignment
    std::ofstream log_file;
    std::mutex log_mutex; // Protects access to log_file
};

#endif // METRICS_H
