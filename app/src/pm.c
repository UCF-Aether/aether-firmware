#include <pm/pm.h>

// pm_constraint_get confusingly checks if the power state is enabled, not if there's a constrain on
// the power state.
void disable_sleep() {
  pm_constraint_set(PM_STATE_SUSPEND_TO_IDLE);
  // pm_constraint_set(PM_STATE_SOFT_OFF);
}

void enable_sleep() {
  pm_constraint_release(PM_STATE_SUSPEND_TO_IDLE);
  // pm_constraint_release(PM_STATE_SOFT_OFF);
}
