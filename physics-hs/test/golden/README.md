# Golden tapes directory

## Status: NO GOLDENS YET

This directory will contain recorded MiniStep baseline JSON when available.

Current blockers:
1. No collision detection → stateOnGround never becomes false after jump
2. accelerateLeft is wrong (unsigned +X, not -X)
3. Apex hang bug (velocity clamps to 0)
4. jumpSquatDuration is magic number (not in PhysicsConfig)
5. B-release zeros X velocity (incorrect)
6. Pose never updated during run

Cannot record valid goldens until these model gaps are fixed.
