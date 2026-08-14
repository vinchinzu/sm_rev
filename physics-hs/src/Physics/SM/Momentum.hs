-- | Extra run speed / momentum (samus_x_extra_run_*).
--
-- Ported from Samus_HandleExtraRunspeedX. Extra run is the residual field
-- that SMB taught us to keep on the observation: planning needs leftover
-- run speed in air, not just the current base table.
module Physics.SM.Momentum
  ( tickExtraRun
  , extraInFacingDir
  ) where

import Physics.SM.Constants
import Physics.SM.Types

-- | Extra run as a signed velocity in the facing direction.
extraInFacingDir :: SamusState -> Velocity
extraInFacingDir state
  | isFacingRight (stateFacing state) = stateXExtra state
  | otherwise = negateVelocity (stateXExtra state)

-- | Build or keep extra run. Matches HandleExtraRunspeedX:
-- build only while running on ground with B in air physics; otherwise
-- keep extra iff momentum is already set (run-jump carry).
tickExtraRun :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
tickExtraRun cfg input state
  | canBuild input state = buildExtra cfg state
  | stateHasMomentum state = state
  | otherwise = state { stateXExtra = zeroVelocity }

canBuild :: ControllerInput -> SamusState -> Bool
canBuild input state =
  stateOnGround state
    && stateEnvironment state == EnvAir
    && stateMovementType state == mvtRunning
    && buttonDown btnB input

buildExtra :: PhysicsConfig -> SamusState -> SamusState
buildExtra cfg state =
  let cap = if equipSpeedBooster (stateEquipment state)
               then cfgExtraRunCapBoost cfg
               else cfgExtraRunCapNormal cfg
      stepped = addVelocity (stateXExtra state) (cfgExtraRunAccel cfg)
      capped = capMagnitude stepped cap
  in state
       { stateXExtra = capped
       , stateHasMomentum = True
       }
