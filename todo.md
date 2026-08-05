# Ideas about Gegelati changes

Wouldn't it be more efficient to have a "list" of evaluation results for every evaluation done with an individual ? (right now its an average automatically).
This could enable more ideas, like some lexicase over the number of evaluation, or compute different score than hand-crafted average (like fitness = minimum score over the episodes, to force consistency if needed)

Sperate seed for algorithm and seed for environment
- we don't know which of the seed actually creates these huge standard deviations.