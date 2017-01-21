#!/bin/bash

echo '
pcm.!default {
  type asym
  playback.pcm "LoopAndReal"
  #capture.pcm "looprec"
  capture.pcm "default"
}

pcm.looprec {
    type hw
    card "Loopback"
    device 1
    subdevice 0
}


pcm.LoopAndReal {
  type plug
  slave.pcm mdev
  route_policy "duplicate"
}


pcm.mdev {
  type multi
  slaves.a.pcm pcm.MixReale
  slaves.a.channels 2
  slaves.b.pcm pcm.MixLoopback
  slaves.b.channels 2
  bindings.0.slave a
  bindings.0.channel 0
  bindings.1.slave a
  bindings.1.channel 1
  bindings.2.slave b
  bindings.2.channel 0
  bindings.3.slave b
  bindings.3.channel 1
}


pcm.MixReale {
  type dmix
  ipc_key 1024
  slave {
    pcm "default"
    #rate 48000
    rate 44100
    periods 128
    period_time 0
    period_size 1024 # must be power of 2
    buffer_size 8192
  }
}

pcm.MixLoopback {
  type dmix
  ipc_key 1025
  slave {
    pcm "hw:Loopback,0,0"
    #rate 48000
    rate 44100
    periods 128
    period_time 0
    period_size 1024 # must be power of 2
    buffer_size 8192
  }
}' > $HOME/.asoundrc