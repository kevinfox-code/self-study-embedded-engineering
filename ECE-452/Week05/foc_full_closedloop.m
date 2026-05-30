%% Full Closed-Loop FOC — GNU Octave  (v7)
%
%  Observer: Closed-Loop Back-EMF Integrator  (corrected angle extraction)
%  ─────────────────────────────────────────────────────────────────────────
%  The total stator flux is:
%    λ_total = L·I  +  λ_PM
%
%  The back-EMF integrator gives λ_total.  To get the rotor (PM) angle we
%  must strip out the inductive term before calling atan2:
%    λ_PM_α = λ_total_α − Ls·iα
%    λ_PM_β = λ_total_β − Ls·iβ
%    θ̂ = atan2(λ_PM_β, λ_PM_α)
%
%  Without this step the angle has a load-dependent offset that grows
%  with current and causes sustained speed ripple and wrong Id/Iq.
%
%  Drift correction (no θ̂ dependency):
%    dλ/dt = (Vαβ − Rs·Iαβ)  +  k·(ψ² − |λ_PM|²)·λ_PM
%  Note: correction uses |λ_PM|, not |λ_total|.
%
%  Run from same directory as svpwm.m

clear; clc;

%% ── Motor parameters ─────────────────────────────────────────────────────
Rs  = 0.5;   Ls  = 2e-3;   % Ld = Lq = Ls (SPM)
psi = 0.05;  p   = 4;
J   = 5e-4;  B   = 1e-4;
Ld  = Ls;    Lq  = Ls;

%% ── Simulation ───────────────────────────────────────────────────────────
Ts    = 1e-4;
t_end = 0.6;
t     = 0 : Ts : t_end;
N     = length(t);
Vdc   = 400;
Vlim  = Vdc / 2;

%% ── Startup timing ───────────────────────────────────────────────────────
t_vf      = 0.08;
t_handoff = 0.10;

%% ── Speed reference ──────────────────────────────────────────────────────
omega_ref_mech = min(max(t - t_handoff, 0) / 0.3, 1) * 1500*(2*pi/60);
omega_ref_elec = omega_ref_mech * p;

%% ── V/f parameters ───────────────────────────────────────────────────────
vf_freq_end  = 20;
vf_omega_end = 2*pi*vf_freq_end;
vf_vmax      = vf_freq_end * 3.0;
vf_vmin      = 4.0;

%% ── Current PI ───────────────────────────────────────────────────────────
wc_i  = 800;
Kp_id = wc_i*Ld;   Ki_id = wc_i*Rs;
Kp_iq = wc_i*Lq;   Ki_iq = wc_i*Rs;

%% ── Speed PI ─────────────────────────────────────────────────────────────
wc_spd = 80;
Kp_spd = J*wc_spd;
Ki_spd = B*wc_spd;
Iq_lim = 15;

%% ── Observer parameters ──────────────────────────────────────────────────
psi_sq  = psi^2;
%  k_drift: restoring gain for |λ_PM| → ψ
%  Rule: k ≈ Rs/(Ls·ψ²)/5  = 0.5/(2e-3*0.0025)/5 = 20000
%  Use lower value for smoother convergence at low speed:
k_drift = 8000;

%% ── PLL ──────────────────────────────────────────────────────────────────
Kp_pll   = 300;
Ki_pll   = 2000;
om_clamp = vf_omega_end * 5;

%% ── Logs ─────────────────────────────────────────────────────────────────
log_id=zeros(1,N);     log_iq=zeros(1,N);
log_id_ref=zeros(1,N); log_iq_ref=zeros(1,N);
log_vd=zeros(1,N);     log_vq=zeros(1,N);
log_va=zeros(1,N);     log_vb=zeros(1,N);
log_theta=zeros(1,N);  log_omega=zeros(1,N);
log_om_m=zeros(1,N);   log_torque=zeros(1,N);
log_ia=zeros(1,N);     log_sector=zeros(1,N);
log_Ta=zeros(1,N);     log_Tb=zeros(1,N);  log_Tc=zeros(1,N);
log_lama=zeros(1,N);   log_lamb=zeros(1,N);
log_lammag=zeros(1,N); log_theta_err=zeros(1,N);

%% ── States ───────────────────────────────────────────────────────────────
id_pl=0; iq_pl=0; th_r=0; om_r=0; om_m=0;

% Observer: total stator flux (initialise = PM flux on α)
lam_a = psi;  lam_b = 0;

th_hat=0; om_hat=0; pll_int=0;
int_id=0; int_iq=0; int_spd=0;
Va_true=0; Vb_true=0;
th_vf=0;
foc_active = false;

%% ── Main loop ────────────────────────────────────────────────────────────
for k = 1:N
  tk = t(k);

  %% PLANT ─────────────────────────────────────────────────────────────
  id_old = id_pl;  iq_old = iq_pl;
  Vd_app =  cos(th_r)*Va_true + sin(th_r)*Vb_true;
  Vq_app = -sin(th_r)*Va_true + cos(th_r)*Vb_true;
  id_pl  = id_old + Ts*(Vd_app - Rs*id_old + om_r*Lq*iq_old)/Ld;
  iq_pl  = iq_old + Ts*(Vq_app - Rs*iq_old - om_r*Ld*id_old - om_r*psi)/Lq;

  Te   = (3/2)*p*(psi*iq_pl + (Ld-Lq)*id_pl*iq_pl);
  om_m = om_m + Ts*(Te - B*om_m)/J;
  om_r = om_m*p;
  th_r = mod(th_r + Ts*om_r + pi, 2*pi) - pi;
  log_om_m(k)=om_m; log_torque(k)=Te;

  %% MEASUREMENTS ──────────────────────────────────────────────────────
  ia_s  =  cos(th_r)*id_pl - sin(th_r)*iq_pl;
  ib_s  =  sin(th_r)*id_pl + cos(th_r)*iq_pl;
  ia_ph =  ia_s;
  ib_ph = -0.5*ia_s + (sqrt(3)/2)*ib_s;
  ic_ph = -0.5*ia_s - (sqrt(3)/2)*ib_s;
  log_ia(k) = ia_ph;

  %% CLARKE ────────────────────────────────────────────────────────────
  i_alpha = (2/3)*(ia_ph - 0.5*ib_ph - 0.5*ic_ph);
  i_beta  = (2/3)*((sqrt(3)/2)*ib_ph - (sqrt(3)/2)*ic_ph);

  %% BACK-EMF FLUX OBSERVER ────────────────────────────────────────────
  %
  %  Step 1: Integrate back-EMF to get total stator flux
  %    λ_total = ∫(Vαβ − Rs·Iαβ)dt
  %
  %  Step 2: Strip inductive component to get PM flux
  %    λ_PM_α = λ_total_α − Ls·iα
  %    λ_PM_β = λ_total_β − Ls·iβ
  %
  %  Step 3: Drift correction on PM flux magnitude
  %    correction = k·(ψ² − |λ_PM|²)·λ_PM
  %    (added at the integrator input so it corrects λ_total)
  %
  %  Step 4: θ̂ = atan2(λ_PM_β, λ_PM_α)

  % PM flux estimate (strip inductive drop from total flux)
  lam_pm_a = lam_a - Ls*i_alpha;
  lam_pm_b = lam_b - Ls*i_beta;

  % Drift correction based on PM flux magnitude
  lam_pm_sq = lam_pm_a^2 + lam_pm_b^2;
  err_flux   = psi_sq - lam_pm_sq;
  corr_a     = k_drift * err_flux * lam_pm_a;
  corr_b     = k_drift * err_flux * lam_pm_b;

  % Integrate total flux
  bemf_a = Va_true - Rs*i_alpha;
  bemf_b = Vb_true - Rs*i_beta;
  lam_a  = lam_a + Ts*(bemf_a + corr_a);
  lam_b  = lam_b + Ts*(bemf_b + corr_b);

  log_lama(k)   = lam_pm_a;
  log_lamb(k)   = lam_pm_b;
  log_lammag(k) = sqrt(lam_pm_sq);

  %% PLL tracks PM flux angle ──────────────────────────────────────────
  th_obs  = atan2(lam_pm_b, lam_pm_a);
  e_pll   = sin(th_obs - th_hat);
  pll_int = max(-om_clamp, min(om_clamp, pll_int + Ki_pll*e_pll*Ts));
  om_hat  = Kp_pll*e_pll + pll_int;
  th_hat  = mod(th_hat + om_hat*Ts + pi, 2*pi) - pi;
  log_theta(k) = th_hat;
  log_omega(k) = om_hat;

  % Angle error vs true rotor angle (diagnostic)
  log_theta_err(k) = mod(th_hat - th_r + pi, 2*pi) - pi;

  %% CONTROL MODE ──────────────────────────────────────────────────────
  if tk < t_handoff

    %% V/f open-loop
    vf_frac  = min(tk/t_vf, 1.0);
    vf_omega = vf_frac * vf_omega_end;
    th_vf    = mod(th_vf + vf_omega*Ts + pi, 2*pi) - pi;
    vf_vmag  = vf_vmin + vf_frac*(vf_vmax - vf_vmin);

    V_alpha = vf_vmag*cos(th_vf);
    V_beta  = vf_vmag*sin(th_vf);

    log_id(k)=0; log_iq(k)=0; log_id_ref(k)=0; log_iq_ref(k)=0;
    log_vd(k)=vf_vmag; log_vq(k)=0;

    if tk >= t_handoff - Ts
      % Warm-start PLL from observer angle (more accurate than th_vf now)
      th_hat  = th_obs;
      pll_int = om_hat;
      foc_active = true;
      fprintf('Handoff t=%.0f ms | th_obs=%6.1f deg | th_r=%6.1f deg | err=%5.1f deg\n', ...
              tk*1e3, rad2deg(th_obs), rad2deg(th_r), rad2deg(th_obs-th_r));
      fprintf('  |lam_PM|=%.4f Wb (target psi=%.4f Wb) | om_hat=%.1f rad/s\n', ...
              sqrt(lam_pm_sq), psi, om_hat);
    end

  else
    %% Closed-loop FOC

    % Park transform
    id_meas =  cos(th_hat)*i_alpha + sin(th_hat)*i_beta;
    iq_meas = -sin(th_hat)*i_alpha + cos(th_hat)*i_beta;
    log_id(k)=id_meas; log_iq(k)=iq_meas;

    % Speed PI
    e_spd   = omega_ref_elec(k) - om_hat;
    int_spd = int_spd + Ki_spd*e_spd*Ts;
    iq_ref  = max(-Iq_lim, min(Iq_lim, Kp_spd*e_spd + int_spd));
    id_ref  = 0;
    log_id_ref(k)=id_ref; log_iq_ref(k)=iq_ref;

    % Current PI
    e_id   = id_ref - id_meas;
    int_id = max(-Vlim, min(Vlim, int_id + Ki_id*e_id*Ts));
    Vd_pi  = Kp_id*e_id + int_id;

    e_iq   = iq_ref - iq_meas;
    int_iq = max(-Vlim, min(Vlim, int_iq + Ki_iq*e_iq*Ts));
    Vq_pi  = Kp_iq*e_iq + int_iq;

    % Decoupling + voltage limit
    Vd = Vd_pi - om_hat*Lq*iq_meas;
    Vq = Vq_pi + om_hat*Ld*id_meas + om_hat*psi;
    Vm = sqrt(Vd^2+Vq^2);
    if Vm > Vlim; Vd=Vd*(Vlim/Vm); Vq=Vq*(Vlim/Vm); end
    log_vd(k)=Vd; log_vq(k)=Vq;

    % Inverse Park
    V_alpha = cos(th_hat)*Vd - sin(th_hat)*Vq;
    V_beta  = sin(th_hat)*Vd + cos(th_hat)*Vq;

  end

  log_va(k)=V_alpha; log_vb(k)=V_beta;

  %% SVPWM ─────────────────────────────────────────────────────────────
  [Ta,Tb,Tc,sector] = svpwm(V_alpha, V_beta, Vdc);
  log_Ta(k)=Ta; log_Tb(k)=Tb; log_Tc(k)=Tc; log_sector(k)=sector;

  %% INVERTER → Vαβ for next step ───────────────────────────────────────
  Va_out  = (Ta-0.5)*Vdc;
  Vb_out  = (Tb-0.5)*Vdc;
  Vc_out  = (Tc-0.5)*Vdc;
  Va_true = (2/3)*(Va_out - 0.5*Vb_out - 0.5*Vc_out);
  Vb_true = (2/3)*((sqrt(3)/2)*Vb_out - (sqrt(3)/2)*Vc_out);

end

%% ── Plots ────────────────────────────────────────────────────────────────
t_ms  = t*1e3;
th_ms = t_handoff*1e3;

figure(1); clf;

subplot(3,3,1);
  plot(t_ms, log_om_m*(60/(2*pi)),'b', ...
       t_ms, omega_ref_mech*(60/(2*pi)),'r--','LineWidth',1.2);
  xline(th_ms,'k:','FOC on');
  xlabel('Time (ms)'); ylabel('Speed (rpm)');
  title('Mechanical speed vs reference');
  legend('\omega_m','\omega_{ref}','Location','southeast'); grid on;

subplot(3,3,2);
  plot(t_ms,log_id,'b',  t_ms,log_id_ref,'b--', ...
       t_ms,log_iq,'r',  t_ms,log_iq_ref,'r--','LineWidth',1);
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('Current (A)');
  title('dq currents vs references');
  legend('Id','Id_{ref}','Iq','Iq_{ref}'); grid on;

subplot(3,3,3);
  plot(t_ms,rad2deg(log_theta_err),'b','LineWidth',1);
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('Error (deg)');
  title('Angle error \hat{\theta} - \theta_r (target: 0)'); grid on;
  yline(0,'r--');

subplot(3,3,4);
  plot(t_ms,log_lama,'b', t_ms,log_lamb,'r','LineWidth',1);
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('Flux (Wb)');
  title('PM flux \lambda_{PM\alpha}, \lambda_{PM\beta}');
  legend('\lambda_{PM\alpha}','\lambda_{PM\beta}'); grid on;

subplot(3,3,5);
  plot(t_ms,log_lammag,'b','LineWidth',1);
  yline(psi,'r--','\psi');
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('|\lambda_{PM}| (Wb)');
  title('PM flux magnitude (target: \psi = 0.05 Wb)'); grid on;

subplot(3,3,6);
  plot(t_ms,log_torque,'b','LineWidth',1);
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('Torque (N·m)');
  title('Electromagnetic torque'); grid on;

subplot(3,3,7);
  plot(t_ms,rad2deg(log_theta),'b','LineWidth',1);
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('Angle (deg)');
  title('\hat{\theta} (cycles ±180° when spinning)'); grid on;

subplot(3,3,8);
  plot(t_ms,log_iq,'r', t_ms,log_iq_ref,'r--','LineWidth',1);
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('Current (A)');
  title('Iq vs Iq_{ref} (torque-producing current)');
  legend('Iq','Iq_{ref}'); grid on;

subplot(3,3,9);
  plot(t_ms,log_ia,'b','LineWidth',1);
  xline(th_ms,'k:');
  xlabel('Time (ms)'); ylabel('Current (A)');
  title('Phase-A current Ia'); grid on;

sgtitle('FOC — Back-EMF observer with inductive-drop correction (v7)');
