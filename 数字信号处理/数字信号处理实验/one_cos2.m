function [x,t] = one_cos(A,w,z,dur) 
t = 0:pi/(10*w):dur;
x = A*cos(w*t+z);
plot(t,x,'ro-')
