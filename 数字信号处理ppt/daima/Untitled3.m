clear ;close all;
[xn,Fs]=audioread('SunshineSquare.wav');Y=fft(xn);
len=length(xn);l=0:len-1;f=(2*pi/len)*l;t=l/Fs;H=ones(1,119071);
figure(1);
subplot(2,1,1);plot(t,xn);title('time');
subplot(2,1,2);plot(f/pi,20*log10(abs(Y)));title('frequency');
%设计巴特沃斯带阻滤波器
ff=[0.2857 0.5714 0.8571];
for i=1:3
    wp=[ff(i)-0.02 ff(i)+0.02];
    ws=[ff(i)-0.01 ff(i)+0.01];
    [n,wc]=buttord(wp,ws,0.1,10);
    [b,a]=butter(n,wc,'stop');
    xn=filter(b,a,xn);
    xn=filter(b,a,xn);
    [H1,~]=freqz(b,a,f);
    H=H.*H1;
end
%设计巴特沃斯高通滤波器
wp =0.01;   
ws =0.02;
[n,wc] = buttord(wp,ws,0.1,10);
[b,a]=butter(n,wc,'high');
xn=filter(b,a,xn);
xn=filter(b,a,xn);
[H1,~]=freqz(b,a,f);
 H=H.*H1;
%做出滤波后的图像
figure(2);
subplot(2,1,1);plot(t,xn);title('time_new');
subplot(2,1,2);plot(f/pi,20*log10(abs(fft(xn))));title('frequency_new');
%做出滤波器的图像
figure(3);
plot(f/pi,abs(H));title("fliter");
soundsc(xn,Fs);