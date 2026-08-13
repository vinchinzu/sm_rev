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
  [ testCase "Ground run RIGHT 60f" $ do
      exists <- doesFileExist "test/golden/run_right_60f.json"
      exists @?= True

  , testCase "Short hop golden exists" $ do
      exists <- doesFileExist "test/golden/short_hop.json"
      exists @?= True

  , testCase "Full hop golden exists" $ do
      exists <- doesFileExist "test/golden/full_hop.json"
      exists @?= True
  ]
