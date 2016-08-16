require 'cosmos/conversions/conversion'
module Cosmos
  class MotorPosition < Conversion
    def call(value, packet, buffer)
	  temp = (packet.read('RAW_CNT_0', :RAW).to_i % 9600) * 0.0375
      if temp < 0
	    temp += 360
	  end
	  return temp.round(4.0)
    end
  end
end