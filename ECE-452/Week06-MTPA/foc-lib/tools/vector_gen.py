#!/usr/bin/env python3
"""
vector_gen.py — Generate test vectors from the float motor model.

Generates committed CSV files in test/vectors/ for:
  - observer_ramp_1krpm.csv   (ramp to 1000 rpm_m at 20 kHz)
  - ident_rs_step.csv         (Rs DC excitation step)

These committed files are used by CI without requiring Python.
Regenerate: python3 tools/vector_gen.py

Model: SM-PMSM, Rs=1.0 Ω, Ls=1e-3 H, λm=5e-3 Wb, pp=7, J=1e-4, B=1e-3.
"""

import math, os, csv

M_PI = math.pi
RS = 1.0; LS = 1e-3; LM = 5e-3; PP = 7
J = 1e-4; B = 1e-3
TS = 50e-6  # 20 kHz
N_RAMP = 2000  # 100 ms of ramp

def wrap(a):
    while a >  M_PI: a -= 2*M_PI
    while a < -M_PI: a += 2*M_PI
    return a

def deriv(s, va, vb, tl):
    ia, ib, th, om = s
    e_a = -LM * om * math.sin(th)
    e_b =  LM * om * math.cos(th)
    dia = (va - RS * ia - e_a) / LS
    dib = (vb - RS * ib - e_b) / LS
    te  = PP * LM * (ib * math.cos(th) - ia * math.sin(th))
    dom = (te - B * om - tl) / J
    dth = om
    return (dia, dib, dth, dom)

def rk4(s, va, vb, tl, dt):
    k1 = deriv(s, va, vb, tl)
    s2 = tuple(s[i] + 0.5*dt*k1[i] for i in range(4))
    k2 = deriv(s2, va, vb, tl)
    s3 = tuple(s[i] + 0.5*dt*k2[i] for i in range(4))
    k3 = deriv(s3, va, vb, tl)
    s4 = tuple(s[i] + dt*k3[i] for i in range(4))
    k4 = deriv(s4, va, vb, tl)
    sn = tuple(s[i] + dt/6*(k1[i]+2*k2[i]+2*k3[i]+k4[i]) for i in range(4))
    return (sn[0], sn[1], wrap(sn[2]), sn[3])

out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "test", "vectors")

# --- observer_ramp_1krpm.csv ---
rows = []
s = (0.0, 0.0, 0.0, 0.0)  # ia, ib, theta, omega
target_rpm = 1000.0
target_omega_e = target_rpm / 60 * 2 * M_PI * PP  # elec rad/s
accel = target_omega_e / (N_RAMP * TS)             # rad/s² elec

for n in range(N_RAMP):
    omega_cmd = min(accel * n * TS * (TS * n), target_omega_e)
    # Simple open-loop V/Hz: Va = vq*cos(th_forced), Vb = vq*sin(th_forced)
    th_forced = 0.5 * accel * (n * TS) ** 2 * PP  # integrated angle
    vq = RS * 0.5 + LM * omega_cmd  # rough V/Hz
    va = -vq * math.sin(th_forced)
    vb =  vq * math.cos(th_forced)
    rows.append({'cycle': n, 'v_alpha': f'{va:.8f}', 'v_beta': f'{vb:.8f}',
                 'i_alpha': f'{s[0]:.8f}', 'i_beta': f'{s[1]:.8f}',
                 'theta_e': f'{s[2]:.8f}', 'omega_e': f'{s[3]:.8f}'})
    s = rk4(s, va, vb, 0.0, TS)

with open(os.path.join(out_dir, 'observer_ramp_1krpm.csv'), 'w', newline='') as f:
    w = csv.DictWriter(f, fieldnames=rows[0].keys())
    w.writeheader(); w.writerows(rows)
print(f"Generated observer_ramp_1krpm.csv ({len(rows)} rows)")

# --- ident_rs_step.csv ---
rows2 = []
s = (0.0, 0.0, 0.0, 0.0)
for n in range(400):  # 20 ms settle + 200 ms avg at 0.3 A
    target_id = 0.3
    # Forced d-axis at theta=0: va drives ia, vb=0
    error = target_id - s[0]
    va = RS * target_id + LS * error / TS  # rough current controller
    va = max(-5.0, min(5.0, va))
    vb = 0.0
    rows2.append({'cycle': n, 'v_alpha': f'{va:.8f}', 'v_beta': f'{vb:.8f}',
                  'i_alpha': f'{s[0]:.8f}', 'i_beta': f'{s[1]:.8f}',
                  'theta_e': f'{s[2]:.8f}', 'omega_e': f'{s[3]:.8f}'})
    s = rk4(s, va, vb, 0.0, TS)

with open(os.path.join(out_dir, 'ident_rs_step.csv'), 'w', newline='') as f:
    w = csv.DictWriter(f, fieldnames=rows2[0].keys())
    w.writeheader(); w.writerows(rows2)
print(f"Generated ident_rs_step.csv ({len(rows2)} rows)")
