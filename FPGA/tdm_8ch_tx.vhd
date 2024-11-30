-- Receiver for 8-channel TDM
-- (c) 2024 Dr.-Ing. Christian Noeding
-- christian@noeding-online.de
-- Released under GNU General Public License v3
-- Source: https://www.github.com/xn--nding-jua/X-FBAPE
--
-- This file contains a TDM-receiver for the Behringer Expansion Cards
-- TDM (Time Division Multiplexed Audio Interface) sends 8 24-bit audio-samples
-- plus 8 additional zero-padding bits right after each other.
-- It uses two additional signals: LR-Clock (Frame Sync) and the serial-clock (bit-clock)
--
-- More information have look at https://gab.wallawalla.edu/~larry.aamodt/engr432/cirrus_logic_TDM_AN301.pdf

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all; 
use ieee.numeric_std.all; -- lib for unsigned and signed

entity tdm_8ch_tx is
	port (
		clk 		: in std_logic; -- bit-clock of TDM signal (for X32 it is 12.288 MHz)
		fsync		: in std_logic; -- Frame sync (for X32 it is 384 kHz)
		ch1_in	: in std_logic_vector(23 downto 0); -- received audio-sample
		ch2_in	: in std_logic_vector(23 downto 0); -- received audio-sample
		ch3_in	: in std_logic_vector(23 downto 0); -- received audio-sample
		ch4_in	: in std_logic_vector(23 downto 0); -- received audio-sample
		ch5_in	: in std_logic_vector(23 downto 0); -- received audio-sample
		ch6_in	: in std_logic_vector(23 downto 0); -- received audio-sample
		ch7_in	: in std_logic_vector(23 downto 0); -- received audio-sample
		ch8_in	: in std_logic_vector(23 downto 0); -- received audio-sample

		sdata		: out std_logic -- serial data (8x 32 bit audio-data: 24 bit of audio followed by 8 zero-bits)
	  );
end tdm_8ch_tx;

architecture rtl of tdm_8ch_tx is
	signal sample_data	: std_logic_vector(32 * 8 - 1 downto 0) := (others=>'0');
	signal bit_cnt			: integer range 0 to (32 * 8) := 0;
	signal zfsync			: std_logic;
begin
	process(clk)
	begin
		if rising_edge(clk) then
			-- check for positive edge of frame-sync
			if fsync = '1' and zfsync = '0' then
				-- we are at the last bit of the last channel
				-- on next falling clock we will start sending with the the MSB of first channel

				-- copy channel-data to output-register
				sample_data(sample_data'high - 1 downto sample_data'high - 1 - 23 - (32 * 1)) <= ch1_in;
				sample_data(sample_data'high - 1 - (32 * 1) downto sample_data'high - 1 - 23 - (32 * 1)) <= ch2_in;
				sample_data(sample_data'high - 1 - (32 * 2) downto sample_data'high - 1 - 23 - (32 * 2)) <= ch3_in;
				sample_data(sample_data'high - 1 - (32 * 3) downto sample_data'high - 1 - 23 - (32 * 3)) <= ch4_in;
				sample_data(sample_data'high - 1 - (32 * 4) downto sample_data'high - 1 - 23 - (32 * 4)) <= ch5_in;
				sample_data(sample_data'high - 1 - (32 * 5) downto sample_data'high - 1 - 23 - (32 * 5)) <= ch6_in;
				sample_data(sample_data'high - 1 - (32 * 6) downto sample_data'high - 1 - 23 - (32 * 6)) <= ch7_in;
				sample_data(sample_data'high - 1 - (32 * 7) downto sample_data'high - 1 - 23 - (32 * 7)) <= ch8_in;
				
				bit_cnt <= 0;
			end if;
			zfsync <= fsync;
		end if;

		if falling_edge(clk) then
			-- continuously outputing data from output-register
			sdata <= sample_data(sample_data'high - 1 - bit_cnt);

			-- increment bit-counter
			bit_cnt <= bit_cnt + 1;
		end if;
	end process;
end rtl;
        