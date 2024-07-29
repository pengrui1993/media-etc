#!/bin/bash
#main pcm0 pcm_enc pcm_struct
RUN_EXEC=pcm_enc
make $RUN_EXEC
\./$RUN_EXEC ./*.pcm


 #ffplay -f s16le -ac 2 -ar 44100 theshow.pcm
#ffplay -f s16le -ch_layout stereo -ar 44100 theshow.pcm
#ffplay -f s16le -ch_layout stereo -ar 44100 theshow.pcm
 
 #ffmpeg -hide_banner -f s16lels -ar 44100 -ac 2 -c:a pcm_s16le -i theshow.pcm out.mp3 -y
 #ffmpeg -f f32le -ar 44100 -ac 2 -c:a pcm_s16le -i theshow.pcm out.mp3 -y

 #ffmpeg -f s16le -ar 44100 -ac 2 -c:a pcm_s16le -i theshow.pcm theshow.m4a -y
 #ffplay theshow.m4a
 #ffprobe -hide_banner theshow.m4a
 #ffmpeg -f s16le -ar 44100 -ac 2 -c:a pcm_s16le -i theshow.pcm theshow.aac -y