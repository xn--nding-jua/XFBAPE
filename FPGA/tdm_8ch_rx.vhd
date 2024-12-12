-- Receiver for 8-channel TDM
-- (c) 2024 Dr.-Ing. Christian Noeding
-- christian@noeding-online.de
-- Released under GNU General Public License v3
-- Source: https://www.github.com/xn--nding-jua/xfbape
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

entity tdm_8ch_rx is
	port (
		clk 		: in std_logic; -- bit-clock of TDM signal (for X32 it is 12.288 MHz)
		fsync		: in std_logic; -- Frame sync (for X32 it is 384 kHz)
		sdata		: in std_logic; -- serial data (8x 32 bit audio-data: 24 bit of audio followed by 8 zero-bits)
		
		ch1_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		ch2_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		ch3_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		ch4_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		ch5_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		ch6_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		ch7_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		ch8_out		: out std_logic_vector(23 downto 0); -- received audio-sample
		sync_out		: out std_logic -- new data received successfully
	  );
end tdm_8ch_rx;

architecture rtl of tdm_8ch_rx is
	signal sample_data	: std_logic_vector(32 * 8 - 1 downto 0) := (others=>'0');
	signal zfsync			: std_logic;
	signal copy_data		: std_logic;
begin
	process(clk)
	begin
		if rising_edge(clk) then
			-- check for positive edge of frame-sync
			if fsync = '1' and zfsync = '0' then
				-- we are reading the last bit of the last channel
				-- on next rising clock we will read the MSB of first channel
				-- so we set the copy-flag to copy data on next rising clock
				copy_data <= '1';
			end if;
			zfsync <= fsync;
			
			-- we reach the first bit of the first channel, so copy the audio-samples
			if copy_data = '1' then
				ch1_out <= sample_data(sample_data'high - (32 * 0) downto sample_data'high - (32 * 0) - 23); -- first 24 bits
				ch2_out <= sample_data(sample_data'high - (32 * 1) downto sample_data'high - (32 * 1) - 23); -- next 24 bits with ignoring 8 zero-bits
				ch3_out <= sample_data(sample_data'high - (32 * 2) downto sample_data'high - (32 * 2) - 23); -- next 24 bits with ignoring 8 zero-bits
				ch4_out <= sample_data(sample_data'high - (32 * 3) downto sample_data'high - (32 * 3) - 23); -- next 24 bits with ignoring 8 zero-bits
				ch5_out <= sample_data(sample_data'high - (32 * 4) downto sample_data'high - (32 * 4) - 23); -- next 24 bits with ignoring 8 zero-bits
				ch6_out <= sample_data(sample_data'high - (32 * 5) downto sample_data'high - (32 * 5) - 23); -- next 24 bits with ignoring 8 zero-bits
				ch7_out <= sample_data(sample_data'high - (32 * 6) downto sample_data'high - (32 * 6) - 23); -- next 24 bits with ignoring 8 zero-bits
				ch8_out <= sample_data(sample_data'high - (32 * 7) downto sample_data'high - (32 * 7) - 23); -- next 24 bits with ignoring 8 zero-bits
				
				copy_data <= '0'; -- reset copy-data-flag
				sync_out <= '1'; -- set data-ready-output
			else
				sync_out <= '0';
			end if;
			
			-- continuously reading bit into shift-register
			sample_data <= sample_data(sample_data'high - 1 downto 0) & sdata; -- in TDM first bit is MSB so shift from right to left
		end if;
	end process;
end rtl;
        