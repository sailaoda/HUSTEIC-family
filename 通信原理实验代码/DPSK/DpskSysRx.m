%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  FileName:            DpskSysRx.m
%  Description:         DPSK系统接收机
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  Parameter List:       
%       Output Parameter
%           RecvSigFiltered     接收信号滤波
%           RecvDpskDemod       DPSK解调信号
%           RecvCorr            同步码相关结果
%           RecvSymbolSampled   码元抽样
%           RecvBit             接收到的比特
%       Input Parameter
%           Fs          采样率
%           Rs          码元速率
%           MsgLen      比特数据源长度
%           RecvSig     接收信号

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [RecvSigFiltered,RecvDpskDemod,RecvCorr,RecvSymbolSampled,RecvBit] = DpskSysRx(Fs,Rs,MsgLen,RecvSig)
UpSampleRate=Fs/Rs;
Ts=UpSampleRate; % symbol period
SymbolLen = (MsgLen+6)*2;
CoderConstraint = 7;%  约束长度 
Rolloff=1;
Preamble=[1 1 1 1 0 1 0 1 1 0 0 1 0 0 0 0];
PreambleSig=[-1 -1 -1 -1 +1 -1 +1 -1 -1 +1 +1 -1 +1 +1 +1 +1];
PreambleLen = 16;
FilterSymbolLen = 6;

%% 请完成下述代码实现完整接收机
% RRC filtering
filterDef=fdesign.pulseshaping(UpSampleRate,'Square Root Raised Cosine','Nsym,Beta',FilterSymbolLen,Rolloff);
myFilter = design(filterDef);
myFilter.Numerator=myFilter.Numerator*UpSampleRate;
RecvSigFiltered = conv(myFilter.Numerator,RecvSig);

% differential decoding
RecvDpskDemod = RecvSigFiltered(1:end-Ts).*conj(RecvSigFiltered(Ts+1:end));  

% preamble search
RecvCorr = zeros(1,floor(length(RecvDpskDemod)/Ts));
for k=1:(length(RecvDpskDemod)/Ts)
    RecvCorr(k) = sum(real(RecvDpskDemod(k:Ts:k+PreambleLen*UpSampleRate-1)).*PreambleSig);
end
RecvCorr = abs(RecvCorr);
[~, pos ] = max(RecvCorr); 

% sampled symbol
RecvSymbolSampled = RecvDpskDemod(pos:UpSampleRate:pos+(PreambleLen+SymbolLen)*UpSampleRate-1);    
RecvSymbolSampled = real(sign(RecvSymbolSampled(PreambleLen+1:end)));

% convolutoinal decoding
trel = poly2trellis(CoderConstraint, [171, 133]);
RecvBit = vitdec(RecvSymbolSampled,trel,5*CoderConstraint,'term','unquant');
RecvBit = RecvBit(1:length(RecvBit)-FilterSymbolLen);
end