require 'cosmos/conversions/conversion'
module Cosmos
  class RoundTo < Conversion
    def initialize(places)
      super()
      @places = places
    end
    def call(value, packet, buffer)
      return value.round(@places.to_f)
    end
  end
end