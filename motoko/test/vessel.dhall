let mainVessel = ../vessel.dhall

let additionalDependencies = [] : List Text

in mainVessel
    with dependencies = mainVessel.dependencies # additionalDependencies
