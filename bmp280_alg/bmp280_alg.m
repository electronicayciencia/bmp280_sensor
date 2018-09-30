clear;

t1 = 27504;
t2 = 26435;
t3 = -1000;

p1 = 36477;
p2 = -10685;
p3 = 3024;
p4 = 2855;
p5 = 140;
p6 = -7;
p7 = 15500;
p8 = -14600;
p9 = 6000;

adc_t = 519888;
adc_p = 415148;
t = adc_t;
p = adc_p;

%% FLOATING TEMPERATURE (testing)
t = 519888;  % lectura

% Calibración (dato)
t1 = 27504;
t2 = 26435;
t3 = -1000;

% Escalado
t  = t  / (2^14);
t1 = t1 / (2^10);
t2 = t2 / (2^9);
t3 = t3 / (2^15);

% Cálculo
t    = t - t1;
temp = t * t2 + t^2 * t3;

% Resultado final
temp = temp / 10

% temp = 25.0825


t_fine = var1 + var2;

%% FLOATING TEMPERATURE (works)

t = adc_t;

tf  = t  / (16*1024);
t1f = t1 / 1024;
t2f = t2 / (5*1024);
t3f = t3 / (5*1024*64);

a = tf - t1f;

temp = a * t2f + a^2 * t3f


%% FLOATING PRESSURE
pf = adc_p;

p4f = p4 * 2^4
p5f = p5 / 2^7
p6f = p6 / 2^17

p1f = p1
p2f = p2 / 2^28
p3f = p3 / 2^41

p7f = p7 / 2^4
p8f = p8 / 2^19
p9f = p9 / 2^35


a = 40 * (temp - 25);

dilatacion   = a^2 * p6f + a * p5f + p4f
compensacion = (a^2 * p3f + a * p2f + 1) * p1f

p = ( 2^20 - pf - dilatacion ) * 6250 / compensacion
correccion = p^2 * p9f + p * p8f + p7f

p = p + correccion











