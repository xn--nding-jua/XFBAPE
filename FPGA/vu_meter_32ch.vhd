-- vu-meter
-- (c) 2023 Dr.-Ing. Christian Noeding
-- christian@noeding-online.de
-- Released under GNU General Public License v3
-- Source: https://www.github.com/xn--nding-jua/Audioplayer
--
-- This file converts 24-bit signed audio-data into a 8-bit unsigned VU-meter between -6dBfs and -78dBfs

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity vu_meter_32ch is
	port (
		ch1_in		:	in signed(23 downto 0) := (others=>'0');
		ch2_in		:	in signed(23 downto 0) := (others=>'0');
		ch3_in		:	in signed(23 downto 0) := (others=>'0');
		ch4_in		:	in signed(23 downto 0) := (others=>'0');
		ch5_in		:	in signed(23 downto 0) := (others=>'0');
		ch6_in		:	in signed(23 downto 0) := (others=>'0');
		ch7_in		:	in signed(23 downto 0) := (others=>'0');
		ch8_in		:	in signed(23 downto 0) := (others=>'0');
		ch9_in		:	in signed(23 downto 0) := (others=>'0');
		ch10_in		:	in signed(23 downto 0) := (others=>'0');
		ch11_in		:	in signed(23 downto 0) := (others=>'0');
		ch12_in		:	in signed(23 downto 0) := (others=>'0');
		ch13_in		:	in signed(23 downto 0) := (others=>'0');
		ch14_in		:	in signed(23 downto 0) := (others=>'0');
		ch15_in		:	in signed(23 downto 0) := (others=>'0');
		ch16_in		:	in signed(23 downto 0) := (others=>'0');
		ch17_in		:	in signed(23 downto 0) := (others=>'0');
		ch18_in		:	in signed(23 downto 0) := (others=>'0');
		ch19_in		:	in signed(23 downto 0) := (others=>'0');
		ch20_in		:	in signed(23 downto 0) := (others=>'0');
		ch21_in		:	in signed(23 downto 0) := (others=>'0');
		ch22_in		:	in signed(23 downto 0) := (others=>'0');
		ch23_in		:	in signed(23 downto 0) := (others=>'0');
		ch24_in		:	in signed(23 downto 0) := (others=>'0');
		ch25_in		:	in signed(23 downto 0) := (others=>'0');
		ch26_in		:	in signed(23 downto 0) := (others=>'0');
		ch27_in		:	in signed(23 downto 0) := (others=>'0');
		ch28_in		:	in signed(23 downto 0) := (others=>'0');
		ch29_in		:	in signed(23 downto 0) := (others=>'0');
		ch30_in		:	in signed(23 downto 0) := (others=>'0');
		ch31_in		:	in signed(23 downto 0) := (others=>'0');
		ch32_in		:	in signed(23 downto 0) := (others=>'0');
		sync_in		:	in std_logic := '0';
		
		vu_ch1		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch2		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch3		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch4		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch5		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch6		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch7		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch8		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch9		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch10		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch11		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch12		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch13		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch14		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch15		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch16		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch17		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch18		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch19		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch20		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch21		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch22		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch23		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch24		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch25		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch26		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch27		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch28		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch29		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch30		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch31		:	out std_logic_vector(7 downto 0) := (others=>'0');
		vu_ch32		:	out std_logic_vector(7 downto 0) := (others=>'0')
	);
end vu_meter_32ch;

architecture Behavioral of vu_meter_32ch is
begin
	process(sync_in)
		variable ch1	:	std_logic_vector(23 downto 0);
		variable ch2	:	std_logic_vector(23 downto 0);
		variable ch3	:	std_logic_vector(23 downto 0);
		variable ch4	:	std_logic_vector(23 downto 0);
		variable ch5	:	std_logic_vector(23 downto 0);
		variable ch6	:	std_logic_vector(23 downto 0);
		variable ch7	:	std_logic_vector(23 downto 0);
		variable ch8	:	std_logic_vector(23 downto 0);
		variable ch9	:	std_logic_vector(23 downto 0);
		variable ch10	:	std_logic_vector(23 downto 0);
		variable ch11	:	std_logic_vector(23 downto 0);
		variable ch12	:	std_logic_vector(23 downto 0);
		variable ch13	:	std_logic_vector(23 downto 0);
		variable ch14	:	std_logic_vector(23 downto 0);
		variable ch15	:	std_logic_vector(23 downto 0);
		variable ch16	:	std_logic_vector(23 downto 0);
		variable ch17	:	std_logic_vector(23 downto 0);
		variable ch18	:	std_logic_vector(23 downto 0);
		variable ch19	:	std_logic_vector(23 downto 0);
		variable ch20	:	std_logic_vector(23 downto 0);
		variable ch21	:	std_logic_vector(23 downto 0);
		variable ch22	:	std_logic_vector(23 downto 0);
		variable ch23	:	std_logic_vector(23 downto 0);
		variable ch24	:	std_logic_vector(23 downto 0);
		variable ch25	:	std_logic_vector(23 downto 0);
		variable ch26	:	std_logic_vector(23 downto 0);
		variable ch27	:	std_logic_vector(23 downto 0);
		variable ch28	:	std_logic_vector(23 downto 0);
		variable ch29	:	std_logic_vector(23 downto 0);
		variable ch30	:	std_logic_vector(23 downto 0);
		variable ch31	:	std_logic_vector(23 downto 0);
		variable ch32	:	std_logic_vector(23 downto 0);
	begin
		if rising_edge(sync_in) then
			-- we received a new audio-sample
			
			-- calculate the absolute-value of the samples
			ch1 := std_logic_vector(abs(ch1_in));
			ch2 := std_logic_vector(abs(ch2_in));
			ch3 := std_logic_vector(abs(ch3_in));
			ch4 := std_logic_vector(abs(ch4_in));
			ch5 := std_logic_vector(abs(ch5_in));
			ch6 := std_logic_vector(abs(ch6_in));
			ch7 := std_logic_vector(abs(ch7_in));
			ch8 := std_logic_vector(abs(ch8_in));
			ch9 := std_logic_vector(abs(ch9_in));
			ch10 := std_logic_vector(abs(ch10_in));
			ch11 := std_logic_vector(abs(ch11_in));
			ch12 := std_logic_vector(abs(ch12_in));
			ch13 := std_logic_vector(abs(ch13_in));
			ch14 := std_logic_vector(abs(ch14_in));
			ch15 := std_logic_vector(abs(ch15_in));
			ch16 := std_logic_vector(abs(ch16_in));
			ch17 := std_logic_vector(abs(ch17_in));
			ch18 := std_logic_vector(abs(ch18_in));
			ch19 := std_logic_vector(abs(ch19_in));
			ch20 := std_logic_vector(abs(ch20_in));
			ch21 := std_logic_vector(abs(ch21_in));
			ch22 := std_logic_vector(abs(ch22_in));
			ch23 := std_logic_vector(abs(ch23_in));
			ch24 := std_logic_vector(abs(ch24_in));
			ch25 := std_logic_vector(abs(ch25_in));
			ch26 := std_logic_vector(abs(ch26_in));
			ch27 := std_logic_vector(abs(ch27_in));
			ch28 := std_logic_vector(abs(ch28_in));
			ch29 := std_logic_vector(abs(ch29_in));
			ch30 := std_logic_vector(abs(ch30_in));
			ch31 := std_logic_vector(abs(ch31_in));
			ch32 := std_logic_vector(abs(ch32_in));

			vu_ch1 <= ch1(22) & ch1(21) & ch1(20) & ch1(18) & ch1(16) & ch1(14) & ch1(12) & ch1(10);
			vu_ch2 <= ch2(22) & ch2(21) & ch2(20) & ch2(18) & ch2(16) & ch2(14) & ch2(12) & ch2(10);
			vu_ch3 <= ch3(22) & ch3(21) & ch3(20) & ch3(18) & ch3(16) & ch3(14) & ch3(12) & ch3(10);
			vu_ch4 <= ch4(22) & ch4(21) & ch4(20) & ch4(18) & ch4(16) & ch4(14) & ch4(12) & ch4(10);
			vu_ch5 <= ch5(22) & ch5(21) & ch5(20) & ch5(18) & ch5(16) & ch5(14) & ch5(12) & ch5(10);
			vu_ch6 <= ch6(22) & ch6(21) & ch6(20) & ch6(18) & ch6(16) & ch6(14) & ch6(12) & ch6(10);
			vu_ch7 <= ch7(22) & ch7(21) & ch7(20) & ch7(18) & ch7(16) & ch7(14) & ch7(12) & ch7(10);
			vu_ch8 <= ch8(22) & ch8(21) & ch8(20) & ch8(18) & ch8(16) & ch8(14) & ch8(12) & ch8(10);
			vu_ch9 <= ch9(22) & ch9(21) & ch9(20) & ch9(18) & ch9(16) & ch9(14) & ch9(12) & ch9(10);
			vu_ch10 <= ch10(22) & ch10(21) & ch10(20) & ch10(18) & ch10(16) & ch10(14) & ch10(12) & ch10(10);
			vu_ch11 <= ch11(22) & ch11(21) & ch11(20) & ch11(18) & ch11(16) & ch11(14) & ch11(12) & ch11(10);
			vu_ch12 <= ch12(22) & ch12(21) & ch12(20) & ch12(18) & ch12(16) & ch12(14) & ch12(12) & ch12(10);
			vu_ch13 <= ch13(22) & ch13(21) & ch13(20) & ch13(18) & ch13(16) & ch13(14) & ch13(12) & ch13(10);
			vu_ch14 <= ch14(22) & ch14(21) & ch14(20) & ch14(18) & ch14(16) & ch14(14) & ch14(12) & ch14(10);
			vu_ch15 <= ch15(22) & ch15(21) & ch15(20) & ch15(18) & ch15(16) & ch15(14) & ch15(12) & ch15(10);
			vu_ch16 <= ch16(22) & ch16(21) & ch16(20) & ch16(18) & ch16(16) & ch16(14) & ch16(12) & ch16(10);
			vu_ch17 <= ch17(22) & ch17(21) & ch17(20) & ch17(18) & ch17(16) & ch17(14) & ch17(12) & ch17(10);
			vu_ch18 <= ch18(22) & ch18(21) & ch18(20) & ch18(18) & ch18(16) & ch18(14) & ch18(12) & ch18(10);
			vu_ch19 <= ch19(22) & ch19(21) & ch19(20) & ch19(18) & ch19(16) & ch19(14) & ch19(12) & ch19(10);
			vu_ch20 <= ch20(22) & ch20(21) & ch20(20) & ch20(18) & ch20(16) & ch20(14) & ch20(12) & ch20(10);
			vu_ch21 <= ch21(22) & ch21(21) & ch21(20) & ch21(18) & ch21(16) & ch21(14) & ch21(12) & ch21(10);
			vu_ch22 <= ch22(22) & ch22(21) & ch22(20) & ch22(18) & ch22(16) & ch22(14) & ch22(12) & ch22(10);
			vu_ch23 <= ch23(22) & ch23(21) & ch23(20) & ch23(18) & ch23(16) & ch23(14) & ch23(12) & ch23(10);
			vu_ch24 <= ch24(22) & ch24(21) & ch24(20) & ch24(18) & ch24(16) & ch24(14) & ch24(12) & ch24(10);
			vu_ch25 <= ch25(22) & ch25(21) & ch25(20) & ch25(18) & ch25(16) & ch25(14) & ch25(12) & ch25(10);
			vu_ch26 <= ch26(22) & ch26(21) & ch26(20) & ch26(18) & ch26(16) & ch26(14) & ch26(12) & ch26(10);
			vu_ch27 <= ch27(22) & ch27(21) & ch27(20) & ch27(18) & ch27(16) & ch27(14) & ch27(12) & ch27(10);
			vu_ch28 <= ch28(22) & ch28(21) & ch28(20) & ch28(18) & ch28(16) & ch28(14) & ch28(12) & ch28(10);
			vu_ch29 <= ch29(22) & ch29(21) & ch29(20) & ch29(18) & ch29(16) & ch29(14) & ch29(12) & ch29(10);
			vu_ch30 <= ch30(22) & ch30(21) & ch30(20) & ch30(18) & ch30(16) & ch30(14) & ch30(12) & ch30(10);
			vu_ch31 <= ch31(22) & ch31(21) & ch31(20) & ch31(18) & ch31(16) & ch31(14) & ch31(12) & ch31(10);
			vu_ch32 <= ch32(22) & ch32(21) & ch32(20) & ch32(18) & ch32(16) & ch32(14) & ch32(12) & ch32(10);
		end if;
	end process;
end Behavioral;
