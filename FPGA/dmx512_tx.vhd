----------------------------------------------------------------------
-- File Downloaded from http://www.nandland.com
----------------------------------------------------------------------
-- This file contains the UART Transmitter.  This transmitter is able
-- to transmit 8 bits of serial data, one start bit, one stop bit,
-- and no parity bit.  When transmit is complete tx_done will be
-- driven high for one clock cycle.
--
-- Set Generic g_CLKS_PER_BIT as follows:
-- g_CLKS_PER_BIT = (Frequency of clk)/(Frequency of UART)
-- Example: 10 MHz Clock, 115200 baud UART
-- (10000000)/(115200) = 87
--
-- DMX512 Timings: Standard
-- =================================
-- Breaktime: t_min: 		88us
-- Mark after Break: 		8us
-- Inter Byte Time: 			0us
-- Mark before break time: 0us
-- Refresh-Rate: 				44Hz
--
-- DMX512 Timings: Compatible
-- =================================
-- Breaktime: 					242us
-- Mark after Break: 		100us
-- Inter Byte Time: 			40us
-- Mark before break time: 12us
-- Refresh-Rate: 				23Hz
--
-- DMX512 Timings: Current settings
-- =================================
-- Breaktime: t_min: 		96us
-- Mark after Break: 		9us
-- Inter Byte Time: 			0us
-- Mark before break time: 21us
-- Refresh-Rate: 				44Hz
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
 
entity dmx512_tx is
  generic(
    clk_rate    : integer := 4000000;
    baud_rate   : integer := 250000;
	 breakTime_us : integer := 88;
	 markTime_us : integer := 8;
	 interByteTime_us : integer := 8;
	 markBeforeBreakTime_us : integer := 8
    );
  port (
    clk				: in std_logic;
    byte_in			: in std_logic_vector(7 downto 0);
    byte_rdy		: in std_logic;

    serial_out 	: out std_logic;
	 byteAddr_out	: out unsigned(9 downto 0); -- 0..512 = 513 -> 10-bit
    tx_done   		: out std_logic
    );
end dmx512_tx;
 
 
architecture RTL of dmx512_tx is
	-- values for general controlling
	signal breakCounter : integer range 0 to 2000 := breakTime_us * (clk_rate/1000000);
	signal markCounter : integer range 0 to 2000 := markTime_us * (clk_rate/1000000);
	signal interByteCounter : integer range 0 to 2000 := interByteTime_us * (clk_rate/1000000); -- wait some clocks. 250ns per clock at 4MHz
	signal markBeforeBreakCounter : integer range 0 to 2000 := markBeforeBreakTime_us * (clk_rate/1000000);
	signal byteCounter : integer range 0 to 512 := 0; -- 0=startByte, 1..512 = 512 channelValues

	-- values for sending single byte
	constant g_CLKS_PER_BIT : integer := clk_rate / baud_rate;

	type t_SM_Main is (s_Break, s_Mark,
							 s_Idle, s_TX_Start_Bit, s_TX_Data_Bits, s_TX_Stop_Bit1, s_TX_Stop_Bit2, s_Cleanup,
							 s_interBytePause, s_MarkBeforeBreak, s_readNewData);
	signal r_SM_Main : t_SM_Main := s_Break;

	signal r_Clk_Count : integer range 0 to g_CLKS_PER_BIT-1 := 0;
	signal r_Bit_Index : integer range 0 to 7 := 0;  -- 8 Bits Total
	signal r_TX_Data   : std_logic_vector(7 downto 0) := (others => '0');
begin
 
   
  p_UART_TX : process (clk)
  begin
    if rising_edge(clk) then
         
      case r_SM_Main is
		   when s_Break =>
				serial_out <= '0';         -- Drive Line Low for Break

				-- stay here for at least 88us
				if (breakCounter > 0) then
					breakCounter <= breakCounter - 1;
				else
					breakCounter <= breakTime_us * (clk_rate/1000000); -- reset breakCounter
					
					r_SM_Main <= s_Mark;
				end if;
			
		   when s_Mark =>
				serial_out <= '1';         -- Drive Line High for Idle
				
				-- stay here for at least 8us
				if (markCounter > 0) then
					markCounter <= markCounter - 1;
				else
					markCounter <= markTime_us * (clk_rate/1000000); -- reset markCounter
					
					r_SM_Main <= s_Idle;
				end if;
			
         when s_Idle =>
				serial_out <= '1';         -- Keep/Drive Line High for Idle
				tx_done   <= '0';
				r_Clk_Count <= 0;
				r_Bit_Index <= 0;

				if byte_rdy = '1' then
					r_TX_Data <= byte_in;

					r_SM_Main <= s_TX_Start_Bit;
				end if;

			-- Send out Start Bit. Start bit = 0
			when s_TX_Start_Bit =>
				serial_out <= '0';

				-- Wait g_CLKS_PER_BIT-1 clock cycles for start bit to finish
				if r_Clk_Count < g_CLKS_PER_BIT-1 then
					r_Clk_Count <= r_Clk_Count + 1;
				else
					r_Clk_Count <= 0;

					r_SM_Main   <= s_TX_Data_Bits;
				end if;

			-- Wait g_CLKS_PER_BIT-1 clock cycles for data bits to finish
			when s_TX_Data_Bits =>
				serial_out <= r_TX_Data(r_Bit_Index);

				if r_Clk_Count < g_CLKS_PER_BIT-1 then
					r_Clk_Count <= r_Clk_Count + 1;
				else
					r_Clk_Count <= 0;

					-- Check if we have sent out all bits
					if r_Bit_Index < 7 then
						r_Bit_Index <= r_Bit_Index + 1;

						r_SM_Main   <= s_TX_Data_Bits; -- remain in this step
					else
						r_Bit_Index <= 0;

						r_SM_Main   <= s_TX_Stop_Bit1;
					end if;
				end if;

			-- Send out Stop bit. Stop bit = 2
			when s_TX_Stop_Bit1 =>
				serial_out <= '1';

				-- Wait g_CLKS_PER_BIT-1 clock cycles for Stop bit to finish
				if r_Clk_Count < g_CLKS_PER_BIT-1 then
					r_Clk_Count <= r_Clk_Count + 1;
				else
					r_Clk_Count <= 0;

					r_SM_Main   <= s_TX_Stop_Bit2;
				end if;
		
			-- Send out Stop bit. Stop bit = 2 
			when s_TX_Stop_Bit2 =>
				-- Wait g_CLKS_PER_BIT-1 clock cycles for Stop bit to finish
				if r_Clk_Count < g_CLKS_PER_BIT-1 then
					r_Clk_Count <= r_Clk_Count + 1;
				else
					r_Clk_Count <= 0;

					r_SM_Main   <= s_Cleanup;
				end if;

			-- Stay here only 1 clock
			when s_Cleanup =>
				tx_done   <= '1';

				r_SM_Main <= s_interBytePause;

			when s_interBytePause =>
				tx_done <= '0';

				-- wait between bytes
				if (interByteCounter > 0) then
					interByteCounter <= interByteCounter - 1;
				else
					interByteCounter <= interByteTime_us * (clk_rate/1000000); -- reset interByteCounter

					-- increment byteCounter
					if byteCounter < 512 then
						byteCounter <= byteCounter + 1;
					else
						byteCounter <= 0;
					end if;

					r_SM_Main   <= s_readNewData;
				end if;
 
			when s_readNewData =>
				-- set address for RAM
				byteAddr_out <= to_unsigned(byteCounter, 10);
				
				if (byteCounter>0) then
					-- send next byte
					r_SM_Main <= s_Idle;
				else
					-- end of sequence
					r_SM_Main <= s_MarkBeforeBreak;
				end if;

			when s_MarkBeforeBreak =>
				if (markBeforeBreakCounter > 0) then
					markBeforeBreakCounter <= markBeforeBreakCounter - 1;
				else
					markBeforeBreakCounter <= markBeforeBreakTime_us * (clk_rate/1000000); -- reset markBeforeBreakCounter
				
					r_SM_Main <= s_Break;
				end if;
				
        when others =>
          r_SM_Main <= s_Break;
 
      end case;
    end if;
  end process p_UART_TX;
end RTL;