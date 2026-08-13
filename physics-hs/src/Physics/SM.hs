-- | Tape runner: execute a sequence of inputs and return all states.
module Physics.SM
  ( module Physics.SM.Types
  , module Physics.SM.Constants
  , module Physics.SM.Step
  , runTape
  ) where

import Physics.SM.Constants
import Physics.SM.Step
import Physics.SM.Types

-- | Run a tape of inputs, returning all intermediate states.
--
-- Useful for golden-tape testing against C oracle outputs.
runTape :: PhysicsConfig -> SamusState -> [ControllerInput] -> [SamusState]
runTape cfg initialSt inputs =
  scanl (flip (step cfg)) initialSt inputs
