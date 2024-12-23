-- Stereo IIR audio-filter
-- (c) 2023 Dr.-Ing. Christian Noeding
-- christian@noeding-online.de
-- Released under GNU General Public License v3
-- Source: https://www.github.com/xn--nding-jua/xfbape
-- Original Source: https://github.com/YetAnotherElectronicsChannel/FPGA-Audio-IIR
--
-- This file contains a stereo IIR audio-filter for low-/high-pass-filter,
-- peak-filter (PEQ), low-/high-shelf-, bandpass- and notch-filter
-- The type of filter can be adjusted by the filter-coefficients
-- It uses Fixed-Point-Multiplication for the coefficients in Qx-format.
-- The audio-bit-depth can be adjusted using generic setup
--
-- Online-Calculation of Filter-Coefficients: https://www.earlevel.com/main/2021/09/02/biquad-calculator-v3

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

entity filter_iir_stereo is
  generic(
    bit_width	:	natural range 24 to 48 := 24;
    coeff_bits	:	natural range 16 to 48 := 40; -- for a Qfract_bits-coefficient: signed-bit + integer-bits + Qfract_bits-bits = 1 + 4 + fract_bits = 40 bit
    fract_bits	:	natural range 16 to 48 := 35
    );
  port (
	clk			:	in std_logic := '0';
	input_l		:	in signed(bit_width - 1 downto 0) := (others=>'0');
	input_r		:	in signed(bit_width - 1 downto 0) := (others=>'0');
	sync_in		:	in std_logic := '0';
	rst			:	in std_logic := '0';
	bypass		:	in std_logic := '0';

	-- coefficients have to be multiplied with 2^fract_bits before
	a0 		:	in signed(coeff_bits - 1 downto 0);
	a1 		:	in signed(coeff_bits - 1 downto 0);
	a2 		:	in signed(coeff_bits - 1 downto 0);

	-- coefficients have to be multiplied with 2^fract_bits before
	b1 		:	in signed(coeff_bits - 1 downto 0);
	b2 		:	in signed(coeff_bits - 1 downto 0);

	output_l		:	out signed(bit_width - 1 downto 0) := (others=>'0');
	output_r		:	out signed(bit_width - 1 downto 0) := (others=>'0');
	sync_out		:	out std_logic := '0'
	);
end filter_iir_stereo;

architecture Behavioral of filter_iir_stereo is
	signal state		:	natural range 0 to 13 := 0;
	
	--signals for multiplier
	signal mult_in_a	:	signed(coeff_bits - 1 downto 0) := (others=>'0');
	signal mult_in_b	:	signed(coeff_bits + bit_width - 1 downto 0) := (others=>'0');
	signal mult_out	:	signed((coeff_bits + coeff_bits + bit_width - 1) downto 0) := (others=>'0');

	--temp regs and delay regs
	signal temp_in_l, in_z1_l, in_z2_l			:	signed(bit_width - 1 downto 0):= (others=>'0');	
	signal temp_in_r, in_z1_r, in_z2_r			:	signed(bit_width - 1 downto 0):= (others=>'0');	
	signal out_z1_l, out_z2_l						:	signed(coeff_bits + bit_width - 1 downto 0):= (others=>'0');	
	signal out_z1_r, out_z2_r						:	signed(coeff_bits + bit_width - 1 downto 0):= (others=>'0');	
	signal temp			:	signed(coeff_bits + bit_width - 1 + 8 downto 0):= (others=>'0');
begin
	-- multiplier
	process(mult_in_a, mult_in_b)
	begin
		mult_out <= mult_in_a * mult_in_b;
	end process;

	--// calculate filter
	process(clk, rst)
	begin
		if (rst = '1') then
			-- reset internal signals
			temp_in_l <= ( others => '0');
			temp_in_r <= ( others => '0');
			in_z1_l <= ( others => '0');
			in_z2_l <= ( others => '0');
			in_z1_r <= ( others => '0');
			in_z2_r <= ( others => '0');
			out_z1_l <= ( others => '0');
			out_z2_l <= ( others => '0');
			out_z1_r <= ( others => '0');
			out_z2_r <= ( others => '0');
			temp <= ( others => '0');
			
			-- set output to zero
			output_l <= ( others => '0');
			output_r <= ( others => '0');
			sync_out <= '1';
			
			-- call last state to reset filter-states for next calculation
			state <= 13;
		else
			if rising_edge(clk) then
				if (sync_in = '1' and state = 0) then
					mult_in_a <= a0;
					mult_in_b <= resize(input_l, coeff_bits + bit_width);
					temp_in_l <= input_l;
					temp_in_r <= input_r;
					state <= 1; -- start of state-machine
				elsif (state = 1) then
					temp <= resize(mult_out, temp'length);
					mult_in_a <= a1;
					mult_in_b <= resize(in_z1_l, coeff_bits + bit_width);
					state <= state + 1;
				elsif (state = 2) then
					-- save and sum up result of (in_z1*a1) to temp and load multiplier with in_z2 and a2
					temp <= temp + resize(mult_out, temp'length);
					mult_in_a <= a2;
					mult_in_b <= resize(in_z2_l, coeff_bits + bit_width);
					state <= state + 1;
								
				elsif (state = 3) then
					-- save and sum up result of (in_z2*a2) to temp and load multiplier with out_z1 and b1
					temp <= temp + resize(mult_out, temp'length);
					mult_in_a <= b1;
					mult_in_b <= out_z1_l;
					state <= state + 1;
					
				elsif (state = 4) then
					-- save and sum up (negative) result of (out_z1*b1) and load multiplier with out_z2 and b2
					temp <= temp - resize(shift_right(mult_out, fract_bits), temp'length);
					mult_in_a <= b2;
					mult_in_b <= out_z2_l;
					state <= state + 1;
									
				elsif (state = 5) then
					-- save and sum up (negative) result of (out_z2*b2)
					temp <= temp - resize(shift_right(mult_out, fract_bits), temp'length);
					state <= state + 1;
					
				elsif (state = 6) then
					-- save all delay registers and save result to output
					in_z2_l <= in_z1_l;
					in_z1_l <= temp_in_l;

					out_z2_l <= out_z1_l;
					out_z1_l <= resize(temp, out_z1_l'length); -- save value with fractions to gain higher resolution for this filter
					
					if (bypass = '1') then
						output_l <= input_l;
					else
						output_l <= resize(shift_right(temp, fract_bits), bit_width); -- resize to 24-bit audio
					end if;

					-- load multiplier with a0 * input
					mult_in_a <= a0;
					mult_in_b <= resize(temp_in_r, coeff_bits + bit_width);

					state <= state + 1;
					
				elsif (state = 7) then
					-- save result of (samplein*a0) to temp and load multiplier with in_z1 and a1
					temp <= resize(mult_out, temp'length);
					mult_in_a <= a1;
					mult_in_b <= resize(in_z1_r, coeff_bits + bit_width);
					state <= state + 1;
					
				elsif (state = 8) then
					-- save and sum up result of (in_z1*a1) to temp and load multiplier with in_z2 and a2
					temp <= temp + resize(mult_out, temp'length);
					mult_in_a <= a2;
					mult_in_b <= resize(in_z2_r, coeff_bits + bit_width);
					state <= state + 1;
					
				elsif (state = 9) then
					-- save and sum up result of (in_z2*a2) to temp and load multiplier with out_z1 and b1
					temp <= temp + resize(mult_out, temp'length);
					mult_in_a <= b1;
					mult_in_b <= out_z1_r;
					state <= state + 1;
					
				elsif (state = 10) then
					-- save and sum up (negative) result of (out_z1*b1) and load multiplier with out_z2 and b2
					temp <= temp - resize(shift_right(mult_out, fract_bits), temp'length);
					mult_in_a <= b2;
					mult_in_b <= out_z2_r;
					state <= state + 1;
					
				elsif (state = 11) then
					-- save and sum up (negative) result of (out_z2*b2)
					temp <= temp - resize(shift_right(mult_out, fract_bits), temp'length);
					state <= state + 1;
					
				elsif (state = 12) then
					-- save all delay registers, save result to output and apply ouput-valid signal
					in_z2_r <= in_z1_r;
					in_z1_r <= temp_in_r;

					out_z2_r <= out_z1_r;
					out_z1_r <= resize(temp, out_z1_r'length); -- save value with fractions to gain higher resolution for this filter
					
					if (bypass = '1') then
						output_r <= input_r;
					else
						output_r <= resize(shift_right(temp, fract_bits), bit_width); -- resize to 24-bit audio
					end if;

					sync_out <= '1';
					state <= state + 1;
					
				elsif (state = 13) then
					sync_out <= '0';
					state <= 0;
				end if;
			end if;
		end if;
	end process;
end Behavioral;