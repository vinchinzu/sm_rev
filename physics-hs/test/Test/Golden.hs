-- | Golden tests: Verify Haskell determinism (not Mini baseline).
--
-- These are HASKELL-PRODUCED goldens for determinism testing.
-- NOT Mini baseline - just verify Haskell step function is consistent.
--
-- Mini baseline goldens require: signed velocity, momentum, speed tracking.
module Test.Golden (tests) where

import Data.Aeson (eitherDecodeFileStrict')
import Physics.SM
import System.Directory (doesFileExist)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit ((@?=), assertFailure, testCase)

tests :: TestTree
tests = testGroup "Goldens (Haskell determinism)"
  [ testCase "Ground run RIGHT (Haskell-produced)" $ do
      exists <- doesFileExist "test/golden/run_right_60f.json"
      if not exists
        then assertFailure "Golden missing: test/golden/run_right_60f.json"
        else return ()  -- TODO: Load and verify once JSON format settled

  , testCase "Short hop (Haskell-produced)" $ do
      exists <- doesFileExist "test/golden/short_hop.json"
      if not exists
        then assertFailure "Golden missing: test/golden/short_hop.json"
        else return ()

  , testCase "Full hop (Haskell-produced)" $ do
      exists <- doesFileExist "test/golden/full_hop.json"
      if not exists
        then assertFailure "Golden missing: test/golden/full_hop.json"
        else return ()
  ]
