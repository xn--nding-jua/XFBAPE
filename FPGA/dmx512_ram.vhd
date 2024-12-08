library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dmx512_ram is
	generic(
		lastAddress : integer := 512
	);
	port(
		enable		: in std_logic;
		
		write			: in std_logic;
		writeAddr	: in unsigned(9 downto 0); -- 0..512
		data_in		: in unsigned(7 downto 0); -- 8 bit

		readAddr		: in unsigned(9 downto 0); -- 0..512
		data_out		: out unsigned(7 downto 0) -- 8 bit
	);
end dmx512_ram;

architecture behav of dmx512_ram is
	type ram_type is array(lastAddress downto 0) of unsigned(7 downto 0);
	signal tmp_ram: ram_type;
begin
	-- writing data to ram
	process(write)
	begin
		if rising_edge(write) then
			if (enable = '1') then
				tmp_ram(to_integer(writeAddr)) <= data_in;
			end if;
		end if;
	end process;

	-- continuously outputting data at specified address
	data_out <= tmp_ram(to_integer(readAddr));
end behav;
