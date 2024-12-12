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

entity tdm_tx is
	generic (
		bit_delay		: integer := 1; -- 0 = STANDARD_MODE, +1 = CIRRUS_LOGIC_MODE
		audio_channels	:	natural range 2 to 8 := 8 -- number of expected audio-channels to receive
	);
	port (
		clk 		: in std_logic; -- Master clock
		fsync		: in std_logic; -- Frame sync
		sclk 		: in std_logic; -- serial data clock
		ch1_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch2_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch3_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch4_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch5_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch6_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch7_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch8_in	: in std_logic_vector(23 downto 0); -- audio-sample to transmit

		sdata		: out std_logic -- output serial data
	  );
end tdm_tx;

architecture rtl of tdm_tx is
	signal sample_data					: std_logic_vector(23 downto 0);
	signal fsync_pos_edge				: std_logic;
	signal neg_edge, pos_edge 			: std_logic;
	signal lr_edge							: std_logic;

	signal zfsync, zzfsync, zzzfsync	: std_logic;
	signal zsclk, zzsclk, zzzsclk 	: std_logic;

	signal bit_cnt							: integer range 0 to 32 := 0;
	signal chn_cnt							: integer range 0 to 7 := 0;
begin
	detect_fsync_pos_edge : process(clk)
	begin
		if rising_edge(clk) then
			zfsync <= fsync;
			zzfsync <= zfsync;
			zzzfsync <= zzfsync;
			if zzfsync = '1' and zzzfsync = '0' then
				fsync_pos_edge <= '1';
			else
				fsync_pos_edge <= '0';
			end if;
		end if;
	end process;

	detect_edge : process(clk)
	begin
		if rising_edge(clk) then
			zsclk <= sclk;
			zzsclk <= zsclk;
			zzzsclk <= zzsclk;
			if zzsclk = '1' and zzzsclk = '0' then
				pos_edge <= '1';
			elsif zzsclk = '0' and zzzsclk = '1' then
				neg_edge <= '1';
			else
				pos_edge <= '0';
				neg_edge <= '0';
			end if;
		end if;
	end process;
 
	detect_sample : process(clk)
	begin
		if rising_edge(clk) then
			if fsync_pos_edge = '1' then -- begin of new block (means channel 1)
				bit_cnt <= 0;
				chn_cnt <= 0; -- reset channel-counter to 0
			else
				if pos_edge = '1' then
					if bit_cnt < 32 then
						bit_cnt <= bit_cnt + 1; -- increment bit_cnt until bit 32
					else
						-- bit_cnt is now 32 -> reset bit-counter to 0
						bit_cnt <= 1;
					end if;
				end if;
				
				if neg_edge = '1' then  	
					if (bit_cnt >= bit_delay) and (bit_cnt < (24 + bit_delay)) then
						-- output current bit
						sdata <= sample_data(23 + bit_delay - bit_cnt); -- TDM sends MSB first
					else
						-- we are beyond the 24 audio-bits -> transmit only zeros
						sdata <= '0';
					end if;

					
					if (bit_cnt = 28) then
						-- increase channel counter
						if chn_cnt < (audio_channels-1) then
							chn_cnt <= chn_cnt + 1; -- increase channel-counter on each LRCLK-edge
						else
							chn_cnt <= 0; -- reset to 0 as we are receiving a fsync only every 192 frames
						end if;
					end if;
					
					if bit_cnt = 29 then
						-- we have reached the last zero-bits before new channel-data will start -> copy next channel to transmit

						case chn_cnt is
							when 0 =>
								sample_data <= ch1_in;
							when 1 =>
								sample_data <= ch2_in;
							when 2 =>
								sample_data <= ch3_in;
							when 3 =>
								sample_data <= ch4_in;
							when 4 =>
								sample_data <= ch5_in;
							when 5 =>
								sample_data <= ch6_in;
							when 6 =>
								sample_data <= ch7_in;
							when 7 =>
								sample_data <= ch8_in;
						end case;
					end if;
				end if;
			end if;
		end if;
	end process;
end rtl;
        