library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity dmx512_ram is
	generic(
		lastAddress : integer := 512
	);
	port(
		enable		: in std_logic;
		
		write			: in std_logic;
		writeAddr	: in unsigned(9 downto 0); -- 0..513
		data_in		: in unsigned(7 downto 0); -- 8 bit

		read			: in std_logic;
		readAddr		: in unsigned(9 downto 0); -- 0..513

		data_out		: out unsigned(7 downto 0) -- 8 bit
	);
end dmx512_ram;

architecture behav of dmx512_ram is
	type ram_type is array(0 to lastAddress+1) of unsigned(7 downto 0);
	signal tmp_ram: ram_type;
begin   
	process(write)
	begin
		if rising_edge(write) then
			if (enable = '1') then
				tmp_ram(conv_integer(writeAddr)) <= data_in;
			end if;
		end if;
	end process;

	process(read)
	begin
		if rising_edge(read) then
			if (enable = '1') then
				-- buildin function conv_integer change the type
				-- from std_logic_vector to integer
				data_out <= tmp_ram(conv_integer(readAddr)); 
			end if;
		end if;
	end process;
end behav;
