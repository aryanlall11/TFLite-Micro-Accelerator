`timescale 1 ps / 1 ps

module cmod_a735t_arm
   (led_2bits_tri_o,
    qspi_flash_io0_io,
    qspi_flash_io1_io,
    qspi_flash_io2_io,
    qspi_flash_io3_io,
    qspi_flash_ss_io,
    reset,
    sys_clock,
    usb_uart_rxd,
    usb_uart_txd);

  output [1:0]led_2bits_tri_o;
  inout qspi_flash_io0_io;
  inout qspi_flash_io1_io;
  inout qspi_flash_io2_io;
  inout qspi_flash_io3_io;
  inout qspi_flash_ss_io;
  input reset;
  input sys_clock;
  input usb_uart_rxd;
  output usb_uart_txd;

  wire [1:0]led_2bits_tri_o;
  wire qspi_flash_io0_i;
  wire qspi_flash_io0_io;
  wire qspi_flash_io0_o;
  wire qspi_flash_io0_t;
  wire qspi_flash_io1_i;
  wire qspi_flash_io1_io;
  wire qspi_flash_io1_o;
  wire qspi_flash_io1_t;
  wire qspi_flash_io2_i;
  wire qspi_flash_io2_io;
  wire qspi_flash_io2_o;
  wire qspi_flash_io2_t;
  wire qspi_flash_io3_i;
  wire qspi_flash_io3_io;
  wire qspi_flash_io3_o;
  wire qspi_flash_io3_t;
  wire qspi_flash_ss_i;
  wire qspi_flash_ss_io;
  wire qspi_flash_ss_o;
  wire qspi_flash_ss_t;
  wire reset;
  wire sys_clock;
  wire usb_uart_rxd;
  wire usb_uart_txd;

  //Changes by Aryan (29/8/21)
  wire M3_RESET_OUT;

  wire [1:0] cfg = 2'b01;
  wire [1:0] led;

  assign led_2bits_tri_o[1] = (~M3_RESET_OUT) | led[1];
  assign led_2bits_tri_o[0] = (usb_uart_rxd ^ usb_uart_txd) | led[0];

  ///////////////////////////////

  m3_for_arty_a7 m3_for_arty_a7_i
       (.CFGITCMEN(cfg),
        .CM3_CODE_AXI3_araddr(),
        .CM3_CODE_AXI3_arburst(),
        .CM3_CODE_AXI3_arcache(),
        .CM3_CODE_AXI3_arlen(),
        .CM3_CODE_AXI3_arlock(),
        .CM3_CODE_AXI3_arprot(),
        .CM3_CODE_AXI3_arready(1'b0),
        .CM3_CODE_AXI3_arsize(),
        .CM3_CODE_AXI3_aruser(),
        .CM3_CODE_AXI3_arvalid(),
        .CM3_CODE_AXI3_awaddr(),
        .CM3_CODE_AXI3_awburst(),
        .CM3_CODE_AXI3_awcache(),
        .CM3_CODE_AXI3_awlen(),
        .CM3_CODE_AXI3_awlock(),
        .CM3_CODE_AXI3_awprot(),
        .CM3_CODE_AXI3_awready(1'b0),
        .CM3_CODE_AXI3_awsize(),
        .CM3_CODE_AXI3_awuser(),
        .CM3_CODE_AXI3_awvalid(),
        .CM3_CODE_AXI3_bready(),
        .CM3_CODE_AXI3_bresp(2'b0),
        .CM3_CODE_AXI3_bvalid(1'b0),
        .CM3_CODE_AXI3_rdata(32'b0),
        .CM3_CODE_AXI3_rlast(1'b0),
        .CM3_CODE_AXI3_rready(),
        .CM3_CODE_AXI3_rresp(2'b0),
        .CM3_CODE_AXI3_rvalid(1'b0),
        .CM3_CODE_AXI3_wdata(),
        .CM3_CODE_AXI3_wlast(),
        .CM3_CODE_AXI3_wready(1'b0),
        .CM3_CODE_AXI3_wstrb(),
        .CM3_CODE_AXI3_wvalid(),

        .M3_RESET_OUT(M3_RESET_OUT),
        .ext_clock(),
        .led_2bits_tri_o(led),
        .locked(),
        .qspi_flash_io0_i(qspi_flash_io0_i),
        .qspi_flash_io0_o(qspi_flash_io0_o),
        .qspi_flash_io0_t(qspi_flash_io0_t),
        .qspi_flash_io1_i(qspi_flash_io1_i),
        .qspi_flash_io1_o(qspi_flash_io1_o),
        .qspi_flash_io1_t(qspi_flash_io1_t),
        .qspi_flash_io2_i(qspi_flash_io2_i),
        .qspi_flash_io2_o(qspi_flash_io2_o),
        .qspi_flash_io2_t(qspi_flash_io2_t),
        .qspi_flash_io3_i(qspi_flash_io3_i),
        .qspi_flash_io3_o(qspi_flash_io3_o),
        .qspi_flash_io3_t(qspi_flash_io3_t),
        .qspi_flash_ss_i(qspi_flash_ss_i),
        .qspi_flash_ss_o(qspi_flash_ss_o),
        .qspi_flash_ss_t(qspi_flash_ss_t),
        .reset(reset),
        .sys_clock(sys_clock),
        .usb_uart_rxd(usb_uart_rxd),
        .usb_uart_txd(usb_uart_txd));
  IOBUF qspi_flash_io0_iobuf
       (.I(qspi_flash_io0_o),
        .IO(qspi_flash_io0_io),
        .O(qspi_flash_io0_i),
        .T(qspi_flash_io0_t));
  IOBUF qspi_flash_io1_iobuf
       (.I(qspi_flash_io1_o),
        .IO(qspi_flash_io1_io),
        .O(qspi_flash_io1_i),
        .T(qspi_flash_io1_t));
  IOBUF qspi_flash_io2_iobuf
       (.I(qspi_flash_io2_o),
        .IO(qspi_flash_io2_io),
        .O(qspi_flash_io2_i),
        .T(qspi_flash_io2_t));
  IOBUF qspi_flash_io3_iobuf
       (.I(qspi_flash_io3_o),
        .IO(qspi_flash_io3_io),
        .O(qspi_flash_io3_i),
        .T(qspi_flash_io3_t));
  IOBUF qspi_flash_ss_iobuf
       (.I(qspi_flash_ss_o),
        .IO(qspi_flash_ss_io),
        .O(qspi_flash_ss_i),
        .T(qspi_flash_ss_t));
endmodule
