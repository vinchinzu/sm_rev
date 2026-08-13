module Main (main) where

import Test.Tasty (TestTree, defaultMain, testGroup)
import Test.Golden qualified
import Test.Properties qualified
import Test.Unit qualified

main :: IO ()
main = defaultMain tests

tests :: TestTree
tests = testGroup "Physics.SM"
  [ Test.Unit.tests
  , Test.Properties.tests
  , Test.Golden.tests
  , Test.HopRise.tests
  ]
