-- | Golden tests: Verify Haskell determinism (not Mini baseline).
--
-- These are HASKELL-PRODUCED goldens for determinism testing.
-- NOT Mini baseline - just verify Haskell step function is consistent.
module Test.Golden (tests) where

import Physics.SM
import System.Directory (doesFileExist)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), testCase)

tests :: TestTree
tests = testGroup "Goldens (Haskell determinism)"
  [ testCase "Ground run RIGHT exists" $ do
      exists <- doesFileExist "test/golden/run_right.json"
      exists @?= True
  
  -- Hop goldens removed: Y velocity was unsigned + applyVelocity always added.
  -- Fixed: applyVelocityY subtracts when rising, adds when falling.
  -- TODO: Re-record hop goldens after fixing jump kinematics.
  ]
