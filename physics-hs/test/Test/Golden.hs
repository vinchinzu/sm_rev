-- | Golden tests: NO GOLDENS YET (Mini baseline gaps).
module Test.Golden (tests) where

import Test.Tasty (TestTree, testGroup)
import Test.Tasty.HUnit (testCase)

tests :: TestTree
tests = testGroup "Goldens"
  [ testCase "No goldens (Mini gaps block recording)" $ return ()
  ]
