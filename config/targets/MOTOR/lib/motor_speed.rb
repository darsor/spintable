require 'cosmos/conversions/conversion'
module Cosmos
  class MotorSpeed < Conversion
    def call(value, packet, buffer)
	  raw_0 = packet.read('RAW_CNT_0', :RAW)
	  time_0 = packet.read('TIMESTAMP_0', :CONVERTED)
	  raw_99 = packet.read('RAW_CNT_99', :RAW)
	  time_99 = packet.read('TIMESTAMP_99', :CONVERTED)
	  speed = (raw_99.to_f - raw_0.to_f) / (time_99.to_f - time_0.to_f) * 0.0375 # (delta count / delta time) * degrees per cnt
	  return speed
    end
  end
end