-- 36-channel Audiomixer
-- (c) 2023-2024 Dr.-Ing. Christian Noeding
-- christian@noeding-online.de
-- Released under GNU General Public License v3
-- Source: https://www.github.com/xn--nding-jua/xfbape
--
-- This file contains a multi-channel audiomixer

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity edge_detector is
	port (
		clk			:	in std_logic := '0';
		input			:	in std_logic := '0';
		output		:	out std_logic := '0'
		);
end edge_detector;

architecture Behavioral of edge_detector is
	signal zinput	: std_logic;
	signal pos_edge	: std_logic;
begin
	detect_edge : process(clk)
	begin
		if rising_edge(clk) then
			zinput <= input;
			if input = '1' and zinput = '0' then
				pos_edge <= '1';
			else
				pos_edge <= '0';
			end if;
		end if;
	end process;
	
	output <= pos_edge;
end Behavioral;
