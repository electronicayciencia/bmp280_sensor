%% Script to load and process BMP280 presure readings
%
% Blog Electronica y Ciencia
% https://electronicayciencia.blogspot.com
%
% GitHub
% https://github.com/electronicayciencia/bmp280_sensor
%
% Reinoso G.
% 06/09/2018
%

%% Load and prepare samples

clear

initial_sr = 1;

files = {
%    'D:\tmp\prueba1.dat'; % tiene datos corruptos
%    'D:\tmp\prueba2.dat'; % datos corruptos
    'D:\tmp\baro\prueba3.dat';
    'D:\tmp\baro\prueba4.dat';
    'D:\tmp\baro\prueba5.dat';
    'D:\tmp\baro\prueba10.dat';
    'D:\tmp\baro\prueba11.dat';
    'D:\tmp\baro\prueba12.dat';
    'D:\tmp\baro\prueba13.dat';
    };

x = [];
t = [];
p = [];

for i = 1:length(files)
    a = load(files{i});
    sr = length(a) / (a(length(a),1)-a(1,1));
    a_down = downsample(a,ceil(sr/(2*initial_sr)));
    x = [x;a_down(:,1)];
    t = [t;a_down(:,2)];
    p = [p;a_down(:,3)];
    
    clear a a_down sr
end


%% Set uniform sample rate
final_sr = 0.5;

[pr, xr] = resample(p,x,final_sr,1,3);

% trim some samples from init and end
pr = pr(26:length(pr)-26);
xr = xr(26:length(xr)-26);

% Normalize
prn = pr - mean(pr);
%prn = prn/max(abs(prn));
prn = prn/100; % hPa
xrn = xr - xr(1);

%% Plot waveform

hold off
plot(xrn/(3600*24),prn);
title('Registro de la presión atmosférica');
xlabel('Tiempo (dias)');
ylabel('Variacion de la presión (hPa)');
grid
axis tight
ax = gca;
set(ax, 'XTick', 1:floor(xrn(length(xrn))/(3600*24)))
set(ax, 'YTick', -8:10)
set(ax, 'YLim', [-8 10] )

%% Plot frequency components
freqs = (0:length(prn)-1)*final_sr/length(prn);

ff = fft(prn);
ff = ff(1:(length(ff)+1)/2);

freqs = freqs(1:length(ff));

% Plot FFT components
%  48h ->   5.8 uHz
%  24h ->  11.6 uHz
%  12h ->  23.1 uHz
%   8h ->  34.7 uHz
%   6h ->  46.3 uHz
%   1h -> 277.8 uHz

plot(freqs(1:75)*1e6,abs(ff(1:75)/max(abs(ff))),'-*')
xlabel('\muHz');
ylabel('Potencia relativa');
title('Componentes de frecuencia sin filtrar');
axis tight
grid;
ax = gca;
set(ax, 'XTick', [0:1:100]);


%% Try zero padding

% Pad to get a bin on 24h and 12h
n = 1000;
zlen = n * final_sr/(1/3600) - length(prn);
wf = [blackman(length(prn)).*prn;zeros(floor(zlen-1),1)];
plot(wf)
%
ff2 = fft(wf);
freqs2 = (0:length(wf)-1)*final_sr/length(wf);

ff2 = ff2(1:(length(ff2)+1)/2);

freqs2 = freqs2(1:length(ff2));

plot(freqs2(1:250)*1e6,abs(ff2(1:250)/max(abs(ff2))),'-*')
xlabel('\muHz');
ylabel('Relative power');
title('Unfiltered frequency components');
grid;

%% Filtered reconstruction
% Gaussian band-stop filter to remove tide waves
convfunc = ones(1, length(ff));

sigma = 1.0e-6;

% notch filter 24h period
f = 1/(24*3600);
convfunc = convfunc .* (1-gaussmf(freqs, [sigma f]));

% notch filter 12h period
f = 1/(12*3600);
convfunc = convfunc .* (1-gaussmf(freqs, [sigma f]));

% Experimental: low-pass filter
%f = 1/(3600*2);
%convfunc = convfunc .* (1-gaussmf(freqs, [sigma f]));
%convfunc = convfunc .* (freqs < f);




%
ff_filtered = ff .* convfunc';

% Plot filtered fft
xlimit = 75;
area(freqs(1:xlimit)*1e6,convfunc(1:xlimit),'BaseValue',1,...
    'FaceColor',0.8*[1 1 1],...
    'EdgeColor','none');
hold on
plot(freqs(1:xlimit)*1e6,abs(ff(1:xlimit)/max(abs(ff))), ...
    'LineWidth',1)
plot(freqs(1:xlimit)*1e6,abs(ff_filtered(1:xlimit)/max(abs(ff_filtered))), ...
    'LineWidth',1)
hold off
title('Composición en frecuencia, filtrada');
xlabel('\muHz');
ylabel('Potencia relativa');
grid;
axis tight
ax = gca;
set(ax, 'XTick', [0:1:550]);



%
% Reconstruct back fft
ff_filtered = [ff_filtered; flipud(conj(ff_filtered(2:length(ff_filtered))))];


hold off
plot(xrn/86400,prn, ...
    'color',0.80*[1,1,1], ...
    'LineWidth',1.5)
hold on
plot(xrn/86400,ifft(ff_filtered),...
    'color','blue',...
    'LineWidth',1);
hold off
title('Reconstrucción filtrada');
xlabel('Tiempo (días)');
ylabel('Variación de la presión (hPa)');
grid
axis tight
ax = gca;
set(ax, 'XTick', 1:floor(xrn(length(xrn))/(3600*24)))
set(ax, 'YTick', -8:10)
set(ax, 'YLim', [-8 10] )



%% Spectrogram

% Filter predominant low freq components
ff_filtered = ff;
ff_filtered(1:2048) = 0;
ff_filtered = [ff_filtered; flipud(conj(ff_filtered(2:length(ff_filtered))))];

% Draw spectrogram
spectrogram(ifft(ff_filtered), ...
    hamming(2^11), ...
    2^6, ...
    2^10, ...
    final_sr, ...
    'yaxis', ...
    'MinThreshold',-35)

title 'Espectrograma de la presión atmosférica'
xlabel 'Días'
ylabel 'Frecuencia (mHz)'
colorbar off
set(ax, 'XTick', 1:floor(xrn(length(xrn))/(3600*24)))


%%
% %% High frequency only
% % Gaussian band-stop filter to remove tide waves
% convfunc = ones(1, length(ff));
% 
% sigma = 1.0e-6;
% 
% 
% % high-pass filter
% f = 1/240;
% convfunc = convfunc .* (1-gaussmf(freqs, [sigma f]));
% convfunc = convfunc .* (freqs > f);
% 
% % low-pass filter
% f = 1/20;
% convfunc = convfunc .* (1-gaussmf(freqs, [sigma f]));
% convfunc = convfunc .* (freqs < f);
% 
% 
% %
% ff_filtered = ff .* convfunc';
% 
% 
% 
% % Reconstruct back fft
% ff_filtered = [ff_filtered; flipud(conj(ff_filtered(2:length(ff_filtered))))];
% prn_filtered = 20*ifft(ff_filtered);
% prn_filtered(1:1000) = 0;
% 
% % Noise profile removal filtering
% % Noise spectrum average
% fft_size = 2^15-1; % ~9 hours
% fft_noise = 0;
% for i = 1:fft_size:(length(prn_filtered)-fft_size);
%     fft_noise = fft_noise + abs(fft(prn_filtered(i:(i+fft_size-1))));
% end
% 
% fft_noise = fft_noise(1:(length(fft_noise)+1)/2);
% fft_noise(1:3) = 0;
% fft_noise = fft_noise / max(fft_noise);
% 
% inic = 240000;
% prefilter = prn_filtered(inic:(fft_size+inic-1));
% fft_prefilter = fft(prefilter);
% fft_prefilter = fft_prefilter(1:(length(fft_prefilter)+1)/2);
% fft_prefilter_abs = abs(fft_prefilter);
% fft_prefilter_phase = angle(fft_prefilter);
% 
% fftpref_scale = max(fft_prefilter_abs);
% fft_prefilter_abs = fft_prefilter_abs / fftpref_scale;
% 
% fft_prefilter_abs = fft_prefilter_abs - 0.325*fft_noise;
% 
% fft_prefilter_abs = fft_prefilter_abs * fftpref_scale;
% fft_prefilter=fft_prefilter_abs.*exp(1i*fft_prefilter_phase);
% 
% fft_prefilter = [fft_prefilter; flipud(conj(fft_prefilter(2:length(fft_prefilter))))];
% 
% prn_filtered(inic:(inic+fft_size-1)) = real(ifft(fft_prefilter));
% 
% 
% 
% hold off
% plot(xrn/86400,prn, ...
%     'color',0.80*[1,1,1], ...
%     'LineWidth',1.5)
% hold on
% plot(xrn/86400,prn_filtered,...
%     'color','blue',...
%     'LineWidth',0.5);
% hold off
% title('Reconstrucción filtrada');
% xlabel('Tiempo (días)');
% ylabel('Variación de la presión (hPa)');
% grid
% axis tight
% ax = gca;
% set(ax, 'XTick', 1:floor(xrn(length(xrn))/(3600*24)))
% set(ax, 'YTick', -8:10)
% set(ax, 'YLim', [-8 10] )
