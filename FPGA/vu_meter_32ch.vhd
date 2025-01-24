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

			vu_ch1  <=  ch1(20)  & ch1(17)  & ch1(15)  & ch1(13)  & ch1(12)  & ch1(11)  & ch1(10)  & ch1(9);
			vu_ch2  <=  ch2(20)  & ch2(17)  & ch2(15)  & ch2(13)  & ch2(12)  & ch2(11)  & ch2(10)  & ch2(9);
			vu_ch3  <=  ch3(20)  & ch3(17)  & ch3(15)  & ch3(13)  & ch3(12)  & ch3(11)  & ch3(10)  & ch3(9);
			vu_ch4  <=  ch4(20)  & ch4(17)  & ch4(15)  & ch4(13)  & ch4(12)  & ch4(11)  & ch4(10)  & ch4(9);
			vu_ch5  <=  ch5(20)  & ch5(17)  & ch5(15)  & ch5(13)  & ch5(12)  & ch5(11)  & ch5(10)  & ch5(9);
			vu_ch6  <=  ch6(20)  & ch6(17)  & ch6(15)  & ch6(13)  & ch6(12)  & ch6(11)  & ch6(10)  & ch6(9);
			vu_ch7  <=  ch7(20)  & ch7(17)  & ch7(15)  & ch7(13)  & ch7(12)  & ch7(11)  & ch7(10)  & ch7(9);
			vu_ch8  <=  ch8(20)  & ch8(17)  & ch8(15)  & ch8(13)  & ch8(12)  & ch8(11)  & ch8(10)  & ch8(9);
			vu_ch9  <=  ch9(20)  & ch9(17)  & ch9(15)  & ch9(13)  & ch9(12)  & ch9(11)  & ch9(10)  & ch9(9);
			vu_ch10 <= ch10(20) & ch10(17) & ch10(15) & ch10(13) & ch10(12) & ch10(11) & ch10(10) & ch10(9);
			vu_ch11 <= ch11(20) & ch11(17) & ch11(15) & ch11(13) & ch11(12) & ch11(11) & ch11(10) & ch11(9);
			vu_ch12 <= ch12(20) & ch12(17) & ch12(15) & ch12(13) & ch12(12) & ch12(11) & ch12(10) & ch12(9);
			vu_ch13 <= ch13(20) & ch13(17) & ch13(15) & ch13(13) & ch13(12) & ch13(11) & ch13(10) & ch13(9);
			vu_ch14 <= ch14(20) & ch14(17) & ch14(15) & ch14(13) & ch14(12) & ch14(11) & ch14(10) & ch14(9);
			vu_ch15 <= ch15(20) & ch15(17) & ch15(15) & ch15(13) & ch15(12) & ch15(11) & ch15(10) & ch15(9);
			vu_ch16 <= ch16(20) & ch16(17) & ch16(15) & ch16(13) & ch16(12) & ch16(11) & ch16(10) & ch16(9);
			vu_ch17 <= ch17(20) & ch17(17) & ch17(15) & ch17(13) & ch17(12) & ch17(11) & ch17(10) & ch17(9);
			vu_ch18 <= ch18(20) & ch18(17) & ch18(15) & ch18(13) & ch18(12) & ch18(11) & ch18(10) & ch18(9);
			vu_ch19 <= ch19(20) & ch19(17) & ch19(15) & ch19(13) & ch19(12) & ch19(11) & ch19(10) & ch19(9);
			vu_ch20 <= ch20(20) & ch20(17) & ch20(15) & ch20(13) & ch20(12) & ch20(11) & ch20(10) & ch20(9);
			vu_ch21 <= ch21(20) & ch21(17) & ch21(15) & ch21(13) & ch21(12) & ch21(11) & ch21(10) & ch21(9);
			vu_ch22 <= ch22(20) & ch22(17) & ch22(15) & ch22(13) & ch22(12) & ch22(11) & ch22(10) & ch22(9);
			vu_ch23 <= ch23(20) & ch23(17) & ch23(15) & ch23(13) & ch23(12) & ch23(11) & ch23(10) & ch23(9);
			vu_ch24 <= ch24(20) & ch24(17) & ch24(15) & ch24(13) & ch24(12) & ch24(11) & ch24(10) & ch24(9);
			vu_ch25 <= ch25(20) & ch25(17) & ch25(15) & ch25(13) & ch25(12) & ch25(11) & ch25(10) & ch25(9);
			vu_ch26 <= ch26(20) & ch26(17) & ch26(15) & ch26(13) & ch26(12) & ch26(11) & ch26(10) & ch26(9);
			vu_ch27 <= ch27(20) & ch27(17) & ch27(15) & ch27(13) & ch27(12) & ch27(11) & ch27(10) & ch27(9);
			vu_ch28 <= ch28(20) & ch28(17) & ch28(15) & ch28(13) & ch28(12) & ch28(11) & ch28(10) & ch28(9);
			vu_ch29 <= ch29(20) & ch29(17) & ch29(15) & ch29(13) & ch29(12) & ch29(11) & ch29(10) & ch29(9);
			vu_ch30 <= ch30(20) & ch30(17) & ch30(15) & ch30(13) & ch30(12) & ch30(11) & ch30(10) & ch30(9);
			vu_ch31 <= ch31(20) & ch31(17) & ch31(15) & ch31(13) & ch31(12) & ch31(11) & ch31(10) & ch31(9);
			vu_ch32 <= ch32(20) & ch32(17) & ch32(15) & ch32(13) & ch32(12) & ch32(11) & ch32(10) & ch32(9);
		end if;
	end process;
end Behavioral;
