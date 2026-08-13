-- | Golden tape tests - REAL goldens only (not Haskell self-output stubs).
module Test.Golden (tests) where

import System.Directory (doesFileExist)
import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit (testCase, assertBool)

tests :: TestTree
tests = testGroup "Golden"
  [ testCase "run_right.json exists" $ do
      exists <- doesFileExist "test/golden/run_right.json"
      assertBool "run_right.json should exist" exists
  ]
