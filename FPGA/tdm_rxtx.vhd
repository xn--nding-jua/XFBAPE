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

entity tdm_rxtx is
	generic (
		bit_delay		: integer := 1; -- 0 = STANDARD_MODE, +1 = CIRRY_LOGIC_MODE
		audio_channels	:	natural range 2 to 8 := 8 -- number of expected audio-channels to receive
	);
	port (
		clk 		: in std_logic; -- Master clock
		fsync		: in std_logic; -- Frame sync (asserted when Z subframe is being transmitted)
		sclk 		: in std_logic; -- output serial data clock (AES3-clock)
		sdata		: in std_logic; -- output serial data
		ch1_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch2_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch3_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch4_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch5_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch6_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch7_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		ch8_tx	: in std_logic_vector(23 downto 0); -- audio-sample to transmit
		
		ch1_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		ch2_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		ch3_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		ch4_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		ch5_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		ch6_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		ch7_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		ch8_rx	: out std_logic_vector(23 downto 0); -- received audio-sample
		tdm_out	: out std_logic;
		sync_out	: out std_logic -- new data received successfully
	  );
end tdm_rxtx;

architecture rtl of tdm_rxtx is
	signal sample_data				: std_logic_vector(23 downto 0);
	signal fsync_pos_edge			: std_logic;
	signal neg_edge, pos_edge 		: std_logic;
	signal lr_edge						: std_logic;
	signal rx_sampledata 			: std_logic;

	signal zfsync, zzfsync, zzzfsync	: std_logic;
	signal zsclk, zzsclk, zzzsclk : std_logic;
	signal ch_tx						: std_logic_vector(23 downto 0);

	signal bit_cnt						: integer range 0 to 31 := 0;
	signal chn_cnt						: integer range 0 to 7 := 0;
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
			if fsync_pos_edge = '1' then -- begin of new block (means channel 1 on Ultranet)
				bit_cnt <= 0;
				chn_cnt <= 0; -- reset channel-counter to 0
				sync_out <= '0';
			else
				if pos_edge = '1' then
					if bit_cnt < 31 then
						bit_cnt <= bit_cnt + 1; -- increment bit_cnt until bit 31
					else
						-- increase channel counter
						if chn_cnt < (audio_channels-1) then
							chn_cnt <= chn_cnt + 1; -- increase channel-counter on each LRCLK-edge
						else
							chn_cnt <= 0; -- reset to 0 as we are receiving a fsync only every 192 frames
						end if;
					end if;
				end if;
				
				if neg_edge = '1' then  	
					if bit_cnt = bit_delay then -- data is x cycles after the word strobe
						rx_sampledata <= '1';
					elsif bit_cnt >= 24+bit_delay then -- reached end of sampledata
						rx_sampledata <= '0';
					end if;
				end if;
				
				if bit_cnt = 30 and neg_edge = '1' then
					-- we have reached the unused 8bits at the end of the frame

					-- set output and raise sync-signal when receiving the last channel
					case chn_cnt is
						when 0 =>
							ch1_rx <= sample_data;
							ch_tx <= ch2_tx; -- load next channel for transmitting
						when 1 =>
							ch2_rx <= sample_data;
							ch_tx <= ch3_tx; -- load next channel for transmitting
						when 2 =>
							ch3_rx <= sample_data;
							ch_tx <= ch4_tx; -- load next channel for transmitting
						when 3 =>
							ch4_rx <= sample_data;
							ch_tx <= ch5_tx; -- load next channel for transmitting
						when 4 =>
							ch5_rx <= sample_data;
							ch_tx <= ch6_tx; -- load next channel for transmitting
						when 5 =>
							ch6_rx <= sample_data;
							ch_tx <= ch7_tx; -- load next channel for transmitting
						when 6 =>
							ch7_rx <= sample_data;
							ch_tx <= ch8_tx; -- load next channel for transmitting
						when 7 =>
							ch8_rx <= sample_data;
							ch_tx <= ch1_tx; -- load next channel for transmitting
					end case;
					
					if (chn_cnt = audio_channels-1) then
						-- set output
						sync_out <= '1';
					end if;
				else
					sync_out <= '0';
				end if;
			end if;
		end if;
	end process;

	get_data : process(clk)
	begin
		if rising_edge(clk) then
			-- receive individual bits for audio-data (24 bits)
			if pos_edge = '1' and rx_sampledata = '1' then
				sample_data <= sample_data(sample_data'high - 1 downto 0) & sdata; -- in TDM first bit is MSB so shift from right to left
				tdm_out <= ch_tx(bit_cnt);
			end if;
		end if;
	end process;
end rtl;
        