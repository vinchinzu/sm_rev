-- | Horizontal movement (walk / run / air X).
--
-- Ported from Samus_CalcBaseSpeed_X / NoDecel and the ROM speed tables.
-- SMB residual work showed air X is the first useful expansion after
-- grounded walk/run: do not hold X velocity in air.
module Physics.SM.Run
  ( updateHorizontalMovement
  ) where

import Physics.SM.Constants
import Physics.SM.Momentum (extraInFacingDir)
import Physics.SM.Pose (facingFromDir)
import Physics.SM.SpeedTable
import Physics.SM.Types

-- | Update horizontal position and base X velocity.
updateHorizontalMovement :: PhysicsConfig -> ControllerInput -> SamusState -> SamusState
updateHorizontalMovement _cfg input state
  | stateOnGround state = applyDisplacement (updateGroundX input state)
  | otherwise           = applyDisplacement (updateAirX input state)

updateGroundX :: ControllerInput -> SamusState -> SamusState
updateGroundX input state =
  let dir = xDirection input
      entry = speedEntry (stateEnvironment state) (stateMovementType state)
      facingDir = if isFacingRight (stateFacing state) then 1 else -1
  in case dir of
       0 -> decelerate entry state
       _ | dir /= facingDir && stateXVel state /= zeroVelocity ->
             decelerate entry state
         | dir /= facingDir ->
             accelerate entry dir state { stateFacing = facingFromDir dir }
         | otherwise ->
             accelerate entry dir state

updateAirX :: ControllerInput -> SamusState -> SamusState
updateAirX input state =
  let dir = xDirection input
      entry = speedEntry (stateEnvironment state) (stateMovementType state)
  in case dir of
       0 ->
         -- C zeros base X when no air direction is held. Extra run stays.
         state { stateXVel = zeroVelocity, stateAccelMode = AccelNone }
       _ ->
         accelerate entry dir state { stateFacing = facingFromDir dir }

accelerate :: SpeedEntry -> Int -> SamusState -> SamusState
accelerate entry dir state =
  let delta = if dir < 0 then negateVelocity (seAccel entry) else seAccel entry
      stepped = addVelocity (stateXVel state) delta
      capped = capTowardMax stepped (signedMax entry dir) dir
  in state
       { stateXVel = capped
       , stateAccelMode = if capped == signedMax entry dir then AccelNone else AccelAccelerating
       }

-- | Cap after adding accel. Direction-aware so we do not flip past max.
capTowardMax :: Velocity -> Velocity -> Int -> Velocity
capTowardMax stepped maxV dir =
  let s = toSigned1616 stepped
      m = toSigned1616 maxV
  in if dir >= 0
        then if s > m then maxV else stepped
        else if s < m then maxV else stepped

decelerate :: SpeedEntry -> SamusState -> SamusState
decelerate entry state =
  let current = stateXVel state
      mag = velocityMagnitude current
      decelMag = velocityMagnitude (seDecel entry)
  in if mag <= decelMag
        then state { stateXVel = zeroVelocity, stateAccelMode = AccelNone }
        else
          let negative = toSigned1616 current < 0
              stepped = addVelocity current (if negative then seDecel entry else negateVelocity (seDecel entry))
          in state { stateXVel = stepped, stateAccelMode = AccelDecelerating }

applyDisplacement :: SamusState -> SamusState
applyDisplacement state =
  let total = addVelocity (stateXVel state) (extraInFacingDir state)
      newPos = applyVelocity (stateXPos state) total
  in state { stateXPos = newPos }
