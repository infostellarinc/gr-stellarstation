# gr-stellarstation: GNURadio blocks for interacting with StellarStation

gr-stellarstation is an out-of-tree GNURadio module that provides blocks for easily integrating with the [StellarStation](stellarstation.com) API in GNURadio flowgraphs.

## Installation

### PyBOMBS

PyBOMBs support is currently under construction.

### Source Installation

gr-stellarstation, like most OOT modules, can be built from source using CMake.

First, we need to install gr-stellarstation's sole dependency, [Conan](https://conan.io/). The simplest way to do this is through `pip`:

```bash
$ pip install conan
```

For alternative methods to install Conan, see the [Conan documentation](https://docs.conan.io/en/latest/installation.html).

After installing Conan, run the following commands to add the relevant Conan recipes:

```bash
$ conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
$ conan remote add inexorgame https://api.bintray.com/conan/inexorgame/inexor-conan
$ conan remote add stellarstation https://api.bintray.com/conan/infostellarinc/stellarstation-conan
```

Proceed to install gr-stellarstation normally with CMake:

```bash
$ git clone https://github.com/infostellarinc/gr-stellarstation.git
$ cd gr-stellarstation
$ mkdir build
$ cmake ..
$ make
$ sudo make install
$ sudo ldconfig
```

## Examples

See the [examples directory](examples/) for flowgraphs showing the usage of gr-stellarstation blocks.

## StellarStation API Key

To use most of the blocks in this module, you will need a StellarStation API Key, which is a .json file containing credentials to access the StellarStation API. An API key can be generated in the StellarStation Console.

## Quickstart

A typical use case of gr-stellarstation is direct streaming of complex I/Q data from a satellite tracked by StellarStation to a GNURadio flowgraph in real-time.

gr-stellarstation provides the block **StellarStation IQ Source** for this purpose.

![iq_source_sample](docs/images/iq_source_sample.png)

Note that we've configured the block to connect to *Satellite ID* "73" (corresponding to the weather satellite [NOAA 19](https://en.wikipedia.org/wiki/NOAA-19)) and given it the path of the StellarStation API Key via *StellarStation API Key Filepath*.

When this flowgraph runs, the block opens a stream to the StellarStation API, and converts all IQ packets received to a complex stream that can be used in downstream GNURadio blocks. We can display a live waterfall plot, save the stream to file, demodulate the data, or anything else GNURadio lets us do!

Here are a few more things to keep in mind about our flowgraph:
* The sampling rate of the received stream is satellite-dependent. Currently, most amateur satellites that StellarStation tracks are sampled at 112500Hz.
* If a pass is not currently running, the StellarStation API will return no data, and your flowgraph will not do anything.
* Depending on both the ground station's and the user's internet speed, samples may not be received in realtime. For example, to support streaming 112500Hz raw I/Q data in realtime, the ground station and the user need to support approximately ~1MBps upload and download speed, respectively. If either one is slower, the packets will still arrive in order without data loss, but  will no longer be in realtime.

For other satellites, StellarStation does demodulation directly on the ground station and streams back the demodulated bitstream. For users that wish to access this data, gr-stellarstation provides the **StellarStation Bitstream Source** block.

![bit_source_sample](docs/images/bit_source_sample.png)

## Advanced Usage

The **StellarStation IQ Source** and **StellarStation Bitstream Source** blocks are hierarchical blocks based on the core **StellarStation API Source** block. It is responsible for connecting to the StellarStation API and sending received packets as GNURadio PMTs in [PDU](https://wiki.gnuradio.org/index.php/Guided_Tutorial_Programming_Topics#5.3.1_PDUs) format for downstream blocks to consume.

Being in PDU format, users can take advantage of GNURadio's built-in PDU manipulation blocks, such as PDU Filter, PDU Remove, etc.

The PDUs received contain the following metadata:

|Key|Type|description|
|---|----|-----------|
|FRAMING|uint64|Contains the framing of telemetry data received in this packet. View the `Framing` format from this [API definition](https://github.com/infostellarinc/stellarstation-api/blob/master/api/src/main/proto/stellarstation/api/v1/stellarstation.proto) for the meaning of each framing option.|
|DOWNLINK_FREQUENCY_HZ|uint64|Contains the satellite's downlink frequency.|
|FRAME_HEADER|blob|Contains the frame's header, if any.|

The PDU vector (second element of the PMT Pair) corresponds to the actual telemetry data in raw byte format. What this data represents depends on the packet's Framing.

While the hierarchical **StellarStation IQ Source** and **StellarStation Bitstream Source** blocks can only return data from **IQ** and **BITSTREAM** framings, respectively, we can use the **StellarStation API Source** block directly to work with data from more than one Framing in a single flowgraph.

![api_source_sample](docs/images/api_source_sample.png)

The above flowgraph shows an example of how to use the **StellarStation API Source** to work with a satellite's bitstream, IQ data, and decoded AX.25 packets at the same time.

We use the **PDU Filter** block to specifically pick out IQ data packets, bitstream packets, and AX.25 packets. The **PDU to Stream** block is a utility block included in gr-stellarstation to convert I/Q packets to a complex stream or BITSTREAM packets to a bytestream.
