-- RS232 data encoder
-- (c) 2023-2024 Dr.-Ing. Christian Noeding
-- christian@noeding-online.de
-- Released under GNU General Public License v3
-- Source: https://www.github.com/xn--nding-jua/xfbape
--
-- This file contains a RS232 data-encoder to transmit individual bytes via a rs232-transmitter
-- It is like a signal multiplexer, that will prepare multiple bytes for a serial-transmitter

library ieee;
use ieee.std_logic_1164.all;
use IEEE.NUMERIC_STD.ALL;

entity rs232_encoder is 
	generic(
		clk_rate_hz    : integer := 48000000;
		txd_rate_hz   : integer := 20
	);
	port
	(
		clk		: in std_logic;
		byte1		: in unsigned(7 downto 0); -- Version
		byte2		: in unsigned(7 downto 0); -- info about noisegates
		byte3		: in unsigned(7 downto 0); -- info about dynamic compressors
		byte4		: in unsigned(7 downto 0); -- info about clipping
		vu_left		: in unsigned(7 downto 0); -- VU-Meter Left
		vu_right		: in unsigned(7 downto 0); -- VU-Meter Right
		vu_sub		: in unsigned(7 downto 0); -- VU-Meter Subwoofer
		vu_ch1		: in unsigned(7 downto 0); -- VU-Meter Channel 1
		vu_ch2		: in unsigned(7 downto 0); -- VU-Meter Channel 2
		vu_ch3		: in unsigned(7 downto 0); -- VU-Meter Channel 3
		vu_ch4		: in unsigned(7 downto 0); -- VU-Meter Channel 4
		vu_ch5		: in unsigned(7 downto 0); -- VU-Meter Channel 5
		vu_ch6		: in unsigned(7 downto 0); -- VU-Meter Channel 6
		vu_ch7		: in unsigned(7 downto 0); -- VU-Meter Channel 7
		vu_ch8		: in unsigned(7 downto 0); -- VU-Meter Channel 8
		vu_ch9		: in unsigned(7 downto 0); -- VU-Meter Channel 9
		vu_ch10		: in unsigned(7 downto 0); -- VU-Meter Channel 10
		vu_ch11		: in unsigned(7 downto 0); -- VU-Meter Channel 11
		vu_ch12		: in unsigned(7 downto 0); -- VU-Meter Channel 12
		vu_ch13		: in unsigned(7 downto 0); -- VU-Meter Channel 13
		vu_ch14		: in unsigned(7 downto 0); -- VU-Meter Channel 14
		vu_ch15		: in unsigned(7 downto 0); -- VU-Meter Channel 15
		vu_ch16		: in unsigned(7 downto 0); -- VU-Meter Channel 16
		vu_ch17		: in unsigned(7 downto 0); -- VU-Meter Channel 17
		vu_ch18		: in unsigned(7 downto 0); -- VU-Meter Channel 18
		vu_ch19		: in unsigned(7 downto 0); -- VU-Meter Channel 19
		vu_ch20		: in unsigned(7 downto 0); -- VU-Meter Channel 20
		vu_ch21		: in unsigned(7 downto 0); -- VU-Meter Channel 21
		vu_ch22		: in unsigned(7 downto 0); -- VU-Meter Channel 22
		vu_ch23		: in unsigned(7 downto 0); -- VU-Meter Channel 23
		vu_ch24		: in unsigned(7 downto 0); -- VU-Meter Channel 24
		vu_ch25		: in unsigned(7 downto 0); -- VU-Meter Channel 25
		vu_ch26		: in unsigned(7 downto 0); -- VU-Meter Channel 26
		vu_ch27		: in unsigned(7 downto 0); -- VU-Meter Channel 27
		vu_ch28		: in unsigned(7 downto 0); -- VU-Meter Channel 28
		vu_ch29		: in unsigned(7 downto 0); -- VU-Meter Channel 29
		vu_ch30		: in unsigned(7 downto 0); -- VU-Meter Channel 30
		vu_ch31		: in unsigned(7 downto 0); -- VU-Meter Channel 31
		vu_ch32		: in unsigned(7 downto 0); -- VU-Meter Channel 32
		TX_rdy		: in std_logic := '0'; -- transmitter is ready for next byte
		
		TX_data	:	out unsigned(7 downto 0); -- databyte to transmit
		TX_send	:	out std_logic := '0' -- send data via transmitter
	);
end entity;

architecture Behavioral of rs232_encoder is
	constant PAYLOAD_TO_TX : integer := 39;
	
	type t_SM_Encoder is (s_Idle, s_Send, s_Prepare, s_Wait);
	signal s_SM_Encoder 	: t_SM_Encoder := s_Idle;
	
	signal txd_rate_cnt	: natural range 0 to clk_rate_hz/(2*txd_rate_hz) := 0;
	signal byte_cnt 		: natural range 0 to PAYLOAD_TO_TX + 3 - 1 := 0; -- payload-bytes + 3 frame-bytes
	
	signal tx_next_byte	: std_logic := '0';
	
	type t_bytearray is array(PAYLOAD_TO_TX - 1 downto 0) of unsigned(7 downto 0);
	signal bytearray	: t_bytearray;
	signal PayloadSum	: unsigned(15 downto 0);
begin
	process(clk)
	begin
		if (rising_edge(clk)) then
			case s_SM_Encoder is
				when s_Idle =>
					-- waiting for next send-interval
					TX_send <= '0';
					
					txd_rate_cnt <= txd_rate_cnt + 1;
					if (txd_rate_cnt = ((clk_rate_hz/(2*txd_rate_hz) - 1))) then -- 4MHz / 200000 = 20 Hz, but we are inverting on each clock-edge, so 10 Hz
						-- we reached the user-defined send-interval -> start new frame
						s_SM_Encoder <= s_Prepare;

						-- reset counter for next interval
						txd_rate_cnt <= 0;
					end if;

				when s_Prepare =>
					-- reset byte-counter
					byte_cnt <= 0;
				
					-- copy all bytes to internal signals as data will change during the transmission
					bytearray(0) <= byte1;
					bytearray(1) <= byte2;
					bytearray(2) <= byte3;
					bytearray(3) <= byte4;
					bytearray(4) <= vu_left;
					bytearray(5) <= vu_right;
					bytearray(6) <= vu_sub;
					bytearray(7) <= vu_ch1;
					bytearray(8) <= vu_ch2;
					bytearray(9) <= vu_ch3;
					bytearray(10) <= vu_ch4;
					bytearray(11) <= vu_ch5;
					bytearray(12) <= vu_ch6;
					bytearray(13) <= vu_ch7;
					bytearray(14) <= vu_ch8;
					bytearray(15) <= vu_ch9;
					bytearray(16) <= vu_ch10;
					bytearray(17) <= vu_ch11;
					bytearray(18) <= vu_ch12;
					bytearray(19) <= vu_ch13;
					bytearray(20) <= vu_ch14;
					bytearray(21) <= vu_ch15;
					bytearray(22) <= vu_ch16;
					bytearray(23) <= vu_ch17;
					bytearray(24) <= vu_ch18;
					bytearray(25) <= vu_ch19;
					bytearray(26) <= vu_ch20;
					bytearray(27) <= vu_ch21;
					bytearray(28) <= vu_ch22;
					bytearray(29) <= vu_ch23;
					bytearray(30) <= vu_ch24;
					bytearray(31) <= vu_ch25;
					bytearray(32) <= vu_ch26;
					bytearray(33) <= vu_ch27;
					bytearray(34) <= vu_ch28;
					bytearray(35) <= vu_ch29;
					bytearray(36) <= vu_ch30;
					bytearray(37) <= vu_ch31;
					bytearray(38) <= vu_ch32;
					
					-- calculate the checksum
					PayloadSum <= to_unsigned(0, 16);

					-- go into send-state
					s_SM_Encoder <= s_Send;
				when s_Send =>

					case byte_cnt is
						when 0 =>
							-- start of frame
							TX_data <= to_unsigned(65, 8); -- character "A"
						when PAYLOAD_TO_TX =>
							-- payload part 1
							TX_data <= unsigned(std_logic_vector(PayloadSum)(15 downto 8));
						when PAYLOAD_TO_TX + 1 =>
							-- payload part 2
							TX_data <= unsigned(std_logic_vector(PayloadSum)(7 downto 0));
						when PAYLOAD_TO_TX + 2 =>
							-- end of frame
							TX_data <= to_unsigned(69, 8); -- character "E"
						when others =>
							-- byte 1...end
							TX_data <= bytearray(byte_cnt - 1);
							
							PayloadSum <= PayloadSum + resize(bytearray(byte_cnt - 1), 16);
					end case;

					-- clear to send
					TX_send <= '1';

					if (byte_cnt < (PAYLOAD_TO_TX + 3)) then
						-- increase byte-counter
						byte_cnt <= byte_cnt + 1;
						
						-- go into wait-state and wait for transmitter to transmit next byte
						s_SM_Encoder <= s_Wait;
					else
						-- all data transmitted -> go into idle-state
						s_SM_Encoder <= s_Idle;
					end if;
					
				when s_Wait =>
					-- wait for transmitter

					-- reset clear-flag
					TX_send <= '0';
					
					-- check if we are ready for next byte
					if (TX_rdy = '1') then
						s_SM_Encoder <= s_Send;
					end if;
					
			end case;
		end if;
	end process;
end Behavioral;