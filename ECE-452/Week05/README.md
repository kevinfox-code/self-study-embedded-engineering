# ECE-452 Week05: Dynamic dq Model and Reference Frame Theory

## Objectives

- Connect the Week 5 reading on dynamic dq modeling to the actual simulation artifacts in this folder.
- Explain how the Clarke/Park reference frames support current control and sensorless FOC.
- Show how the observer, PLL, and SVPWM blocks fit together in a closed-loop drive model.

## What Was Built

- `math.md` collects the dq-model equations, observer relationships, and inverter mappings used in the Week 5 notes.
- `foc_full_closedloop.m` runs a GNU Octave closed-loop FOC simulation with a back-EMF flux observer, PLL, speed loop, current loops, and startup handoff logic.
- `svpwm.m` provides the seven-segment SVPWM duty-cycle calculation used by the simulation.

## Key Concepts

- The dq model turns three-phase machine dynamics into two control axes that are easier to regulate independently.
- The observer estimates rotor flux from voltage and current so the controller can track angle without a physical sensor.
- The PLL cleans up the flux angle estimate before it is used by the Park transform and by the decoupling terms.
- SVPWM converts the requested alpha-beta voltage vector into timer-friendly duty cycles for a three-phase inverter.

## Build and Run

This week is simulation-focused, so there is no embedded build target in this folder.

To review the model in GNU Octave, run the simulation from the Week05 directory so `svpwm.m` stays on the path:

```bash
octave --no-gui --quiet foc_full_closedloop.m
```

Expected outcome: the script produces plots for speed, dq current tracking, flux magnitude, torque, angle error, and SVPWM behavior.

## References

- Krishnan, *Permanent Magnet Synchronous and Brushless DC Motor Drives*, Chapter 3, especially the dq model and reference frame derivations.
- Krishnan, Chapter 6, for current-controller design context.
- `ECE-452/ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.md` for the course reading and lab sequence.

## AI Assistance

- Copilot was used to draft and organize this README.
- Human review should confirm that the folder contents, filenames, and week title match the Week 5 course scope.