#include "MinerAux.h"
#include "MsgWndIPC.h"
#include "MinerHelper.h"


bool isTrue(std::string const& _m)
{
	return _m == "on" || _m == "yes" || _m == "true" || _m == "1";
}

bool isFalse(std::string const& _m)
{
	return _m == "off" || _m == "no" || _m == "false" || _m == "0";
}

bool MinerCLI::interpretOption(int& i, int argc, char** argv)
{
	string arg = argv[i];
	if ((arg == "-F" || arg == "--farm") && i + 1 < argc)
	{
		mode = OperationMode::Farm;
		m_farmURL = argv[++i];
		m_activeFarmURL = m_farmURL;
	}
	else if ((arg == "-FF" || arg == "-FS" || arg == "--farm-failover" || arg == "--stratum-failover") && i + 1 < argc)
	{
		string url = argv[++i];

		if (mode == OperationMode::Stratum)
		{
			size_t p = url.find_last_of(":");
			if (p > 0)
			{
				m_farmFailOverURL = url.substr(0, p);
				if (p + 1 <= url.length())
					m_fport = url.substr(p + 1);
			}
			else
			{
				m_farmFailOverURL = url;
			}
		}
		else
		{
			m_farmFailOverURL = url;
		}
	}
	else if (arg == "--farm-recheck" && i + 1 < argc)
		try {
		m_farmRecheckSet = true;
		m_farmRecheckPeriod = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
	else if (arg == "--farm-retries" && i + 1 < argc)
		try {
		m_maxFarmRetries = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
#if ETH_STRATUM || !ETH_TRUE
	else if ((arg == "-S" || arg == "--stratum") && i + 1 < argc)
	{
		mode = OperationMode::Stratum;
		string url = string(argv[++i]);
		size_t p = url.find_last_of(":");
		if (p > 0)
		{
			m_farmURL = url.substr(0, p);
			if (p + 1 <= url.length())
				m_port = url.substr(p + 1);
		}
		else
		{
			m_farmURL = url;
		}
	}
	else if ((arg == "-O" || arg == "--userpass") && i + 1 < argc)
	{
		string userpass = string(argv[++i]);
		size_t p = userpass.find_first_of(":");
		m_user = userpass.substr(0, p);
		if (p + 1 <= userpass.length())
			m_pass = userpass.substr(p + 1);
	}
	else if ((arg == "-SC" || arg == "--stratum-client") && i + 1 < argc)
	{
		try {
			m_stratumClientVersion = atoi(argv[++i]);
			if (m_stratumClientVersion > 2) m_stratumClientVersion = 2;
			else if (m_stratumClientVersion < 1) m_stratumClientVersion = 1;
		}
		catch (...)
		{
			cerr << "Bad " << arg << " option: " << argv[i] << endl;
			BOOST_THROW_EXCEPTION(BadArgument());
		}
	}
	else if ((arg == "-SP" || arg == "--stratum-protocol") && i + 1 < argc)
	{
		try {
			m_stratumProtocol = atoi(argv[++i]);
		}
		catch (...)
		{
			cerr << "Bad " << arg << " option: " << argv[i] << endl;
			BOOST_THROW_EXCEPTION(BadArgument());
		}
	}
	else if ((arg == "-SE" || arg == "--stratum-email") && i + 1 < argc)
	{
		try {
			m_email = string(argv[++i]);
		}
		catch (...)
		{
			cerr << "Bad " << arg << " option: " << argv[i] << endl;
			BOOST_THROW_EXCEPTION(BadArgument());
		}
	}
	else if ((arg == "-FO" || arg == "--failover-userpass") && i + 1 < argc)
	{
		string userpass = string(argv[++i]);
		size_t p = userpass.find_first_of(":");
		m_fuser = userpass.substr(0, p);
		if (p + 1 <= userpass.length())
			m_fpass = userpass.substr(p + 1);
	}
	else if ((arg == "-u" || arg == "--user") && i + 1 < argc)
	{
		m_user = string(argv[++i]);
	}
	else if ((arg == "-p" || arg == "--pass") && i + 1 < argc)
	{
		m_pass = string(argv[++i]);
	}
	else if ((arg == "-o" || arg == "--port") && i + 1 < argc)
	{
		m_port = string(argv[++i]);
	}
	else if ((arg == "-fu" || arg == "--failover-user") && i + 1 < argc)
	{
		m_fuser = string(argv[++i]);
	}
	else if ((arg == "-fp" || arg == "--failover-pass") && i + 1 < argc)
	{
		m_fpass = string(argv[++i]);
	}
	else if ((arg == "-fo" || arg == "--failover-port") && i + 1 < argc)
	{
		m_fport = string(argv[++i]);
	}
	else if ((arg == "--work-timeout") && i + 1 < argc)
	{
		m_worktimeout = atoi(argv[++i]);
	}

#endif
#if ETH_ETHASHCL || !ETH_TRUE
	else if (arg == "--opencl-platform" && i + 1 < argc)
		try {
		m_openclPlatform = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
	else if (arg == "--opencl-devices" || arg == "--opencl-device")
		while (m_openclDeviceCount < 16 && i + 1 < argc)
		{
			try
			{
				m_openclDevices[m_openclDeviceCount] = stol(argv[++i]);
				++m_openclDeviceCount;
			}
			catch (...)
			{
				i--;
				break;
			}
		}
#endif
#if ETH_ETHASHCL || ETH_ETHASHCUDA || !ETH_TRUE
	else if ((arg == "--cl-global-work" || arg == "--cuda-grid-size") && i + 1 < argc)
		try {
		m_globalWorkSizeMultiplier = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
	else if ((arg == "--cl-local-work" || arg == "--cuda-block-size") && i + 1 < argc)
		try {
		m_localWorkSize = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
	else if (arg == "--list-devices")
		m_shouldListDevices = true;
	else if ((arg == "--cl-extragpu-mem" || arg == "--cuda-extragpu-mem") && i + 1 < argc)
		m_extraGPUMemory = 1000000 * stol(argv[++i]);
#endif
#if ETH_ETHASHCL || !ETH_TRUE
	else if (arg == "--allow-opencl-cpu")
		m_clAllowCPU = true;
#endif
#if ETH_ETHASHCUDA || !ETH_TRUE
	else if (arg == "--cuda-devices")
	{
		while (m_cudaDeviceCount < 16 && i + 1 < argc)
		{
			try
			{
				m_cudaDevices[m_cudaDeviceCount] = stol(argv[++i]);
				++m_cudaDeviceCount;
			}
			catch (...)
			{
				i--;
				break;
			}
		}
	}
	else if (arg == "--cuda-schedule" && i + 1 < argc)
	{
		string mode = argv[++i];
		if (mode == "auto") m_cudaSchedule = 0;
		else if (mode == "spin") m_cudaSchedule = 1;
		else if (mode == "yield") m_cudaSchedule = 2;
		else if (mode == "sync") m_cudaSchedule = 4;
		else
		{
			cerr << "Bad " << arg << " option: " << argv[i] << endl;
			BOOST_THROW_EXCEPTION(BadArgument());
		}
	}
	else if (arg == "--cuda-streams" && i + 1 < argc)
		m_numStreams = stol(argv[++i]);
#endif
	else if ((arg == "-L" || arg == "--dag-load-mode") && i + 1 < argc)
	{
		string mode = argv[++i];
		if (mode == "parallel") m_dagLoadMode = DAG_LOAD_MODE_PARALLEL;
		else if (mode == "sequential") m_dagLoadMode = DAG_LOAD_MODE_SEQUENTIAL;
		else if (mode == "single")
		{
			m_dagLoadMode = DAG_LOAD_MODE_SINGLE;
			m_dagCreateDevice = stol(argv[++i]);
		}
		else
		{
			cerr << "Bad " << arg << " option: " << argv[i] << endl;
			BOOST_THROW_EXCEPTION(BadArgument());
		}
	}
	/*
	else if (arg == "--phone-home" && i + 1 < argc)
	{
	string m = argv[++i];
	if (isTrue(m))
	m_phoneHome = true;
	else if (isFalse(m))
	m_phoneHome = false;
	else
	{
	cerr << "Bad " << arg << " option: " << m << endl;
	BOOST_THROW_EXCEPTION(BadArgument());
	}
	}
	*/
	else if (arg == "--benchmark-warmup" && i + 1 < argc)
		try {
		m_benchmarkWarmup = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
	else if (arg == "--benchmark-trial" && i + 1 < argc)
		try {
		m_benchmarkTrial = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
	else if (arg == "--benchmark-trials" && i + 1 < argc)
		try
	{
		m_benchmarkTrials = stol(argv[++i]);
	}
	catch (...)
	{
		cerr << "Bad " << arg << " option: " << argv[i] << endl;
		BOOST_THROW_EXCEPTION(BadArgument());
	}
	else if (arg == "-C" || arg == "--cpu")
		m_minerType = MinerType::CPU;
	else if (arg == "-G" || arg == "--opencl")
		m_minerType = MinerType::CL;
	else if (arg == "-U" || arg == "--cuda")
	{
		m_minerType = MinerType::CUDA;
	}
	else if (arg == "-X" || arg == "--cuda-opencl")
	{
		m_minerType = MinerType::Mixed;
	}
	else if (arg == "-M" || arg == "--benchmark")
	{
		mode = OperationMode::Benchmark;
		if (i + 1 < argc)
		{
			string m = boost::to_lower_copy(string(argv[++i]));
			try
			{
				m_benchmarkBlock = stol(m);
			}
			catch (...)
			{
				if (argv[i][0] == 45) { // check next arg
					i--;
				}
				else {
					cerr << "Bad " << arg << " option: " << argv[i] << endl;
					BOOST_THROW_EXCEPTION(BadArgument());
				}
			}
		}
	}
	else if (arg == "-Z" || arg == "--simulation") {
		mode = OperationMode::Simulation;
		if (i + 1 < argc)
		{
			string m = boost::to_lower_copy(string(argv[++i]));
			try
			{
				m_benchmarkBlock = stol(m);
			}
			catch (...)
			{
				if (argv[i][0] == 45) { // check next arg
					i--;
				}
				else {
					cerr << "Bad " << arg << " option: " << argv[i] << endl;
					BOOST_THROW_EXCEPTION(BadArgument());
				}
			}
		}
	}
	else if ((arg == "-t" || arg == "--mining-threads") && i + 1 < argc)
	{
		try
		{
			m_miningThreads = stol(argv[++i]);
		}
		catch (...)
		{
			cerr << "Bad " << arg << " option: " << argv[i] << endl;
			BOOST_THROW_EXCEPTION(BadArgument());
		}
	}
	else
		return false;
	return true;
}


bool MinerCLI::IsIntegratedGraphics(std::string strDevicesName)
{
	transform(strDevicesName.begin(), strDevicesName.end(), strDevicesName.begin(), tolower);
	if (strDevicesName.find("intel") != std::string::npos)
	{
		return true;
	}
	return false;
}

bool MinerCLI::FilterGpuDevices(uint32_t& platformID, std::vector<uint32_t> &deviceID)
{
	std::vector<ethash_cl_miner::platform_device_info> v_pd_info = ethash_cl_miner::GetAllPlatformDeviceInfo(m_extraGPUMemory, 0);
	if (v_pd_info.size() > 0)
	{
		uint64_t max_global_men_size = 0;
		std::set<uint32_t> set_platform_has_IG, set_platform_max_global_size;
		std::map<uint32_t, std::vector<uint32_t>> platforms;
		typedef std::map<uint32_t, std::vector<uint32_t>>::iterator c_iter;
		for (ethash_cl_miner::platform_device_info const& pd_info : v_pd_info)
		{
			if (IsIntegratedGraphics(pd_info.name))
			{
				set_platform_has_IG.insert(pd_info.platformId);
				continue;
			}
			if (pd_info.global_mem_size > max_global_men_size)
			{
				set_platform_max_global_size.clear();
				set_platform_max_global_size.insert(pd_info.platformId);
				max_global_men_size = pd_info.global_mem_size;
			}
			else if (pd_info.global_mem_size == max_global_men_size)
			{
				set_platform_max_global_size.insert(pd_info.platformId);
			}
			c_iter it_pd = platforms.find(pd_info.platformId);
			if (it_pd == platforms.end())
			{
				std::vector<uint32_t> devices;
				devices.push_back(pd_info.deviceId);
				platforms.insert(make_pair(pd_info.platformId, devices));
			}
			else
			{
				(it_pd->second).push_back(pd_info.deviceId);
			}
		}
		if (set_platform_max_global_size.size() > 0)
		{
			std::set<uint32_t> set_result;
			for (uint32_t const& platform_max_global_size : set_platform_max_global_size)
			{
				if (set_platform_has_IG.find(platform_max_global_size) == set_platform_has_IG.end())
				{
					set_result.insert(platform_max_global_size);
				}
			}
			if (set_result.empty())
			{
				set_result = set_platform_max_global_size;
			}
			unsigned device_num = 0;
			uint32_t platformid = 0;
			for (uint32_t const& result : set_result)
			{
				if (platforms[result].size() > device_num)
				{
					device_num = platforms[result].size();
					platformID = result;

				}
			}
			deviceID = platforms[platformID];
			return true;

		}

	}
	return false;
}

void MinerCLI::InitOpenCLParam()
{
	uint32_t platformID;
	std::vector<uint32_t> deviceID;
	if (!FilterGpuDevices(platformID, deviceID))
	{
		cout << "select Devices failed, No GPU device with sufficient memory was found. Can't GPU mine."<< endl;
		exit(1);
		return;
	}
	if (deviceID.size()> 0)
	{
		for (uint32_t const& udevice : deviceID)
		{
			if (m_openclDeviceCount >= 16)
			{
				break;
			}
			m_openclDevices[m_openclDeviceCount] = udevice;
			++m_openclDeviceCount;
		}
		EthashGPUMiner::setDevices(m_openclDevices, m_openclDeviceCount);
		m_miningThreads = m_openclDeviceCount;
	}
	m_openclPlatform = platformID;
	if (!EthashGPUMiner::configureGPU(
		m_localWorkSize,
		m_globalWorkSizeMultiplier,
		m_msPerBatch,
		m_openclPlatform,
		m_openclDevice,
		m_clAllowCPU,
		m_extraGPUMemory,
		0,
		m_dagLoadMode,
		m_dagCreateDevice
		))
		exit(1);
	EthashGPUMiner::setNumInstances(m_miningThreads);
	PostMessageToUserWnd(WM_MINING_COUNT, m_miningThreads, 0);
}



void MinerCLI::setSpeedLimitParam(unsigned _globalWorkSizeMultiplier, unsigned _msPerBatch)
{
	if (m_pfarm)
	{
		m_pfarm->setSpeedLimitParam(_globalWorkSizeMultiplier, _msPerBatch);
	}
}

void MinerCLI::ProcessDagProgress(int _pc)
{
	if (!m_bCompleteDAGOnce)
	{
		if (_pc == 100)
		{
			m_bCompleteDAGOnce = true;
		}
		{
			dev::WriteGuard l(x_dagprecent);
			if (_pc <= m_iCurrentPrecent)
			{
				return;
			}
			m_iCurrentPrecent = _pc;
		}
		int precent = 0;
		{
			dev::ReadGuard l(x_dagprecent);
			precent = m_iCurrentPrecent;

		}
		MineCalculateDAG mcalc;
		mcalc.uSize = sizeof(MineCalculateDAG);
		mcalc.uType = MineMsgType::MING_CALCULATE_DAG;
		mcalc.uPrecent = precent;
		MsgWndIPC::Instance()->WritePipeData((byte*)&mcalc, sizeof(MineCalculateDAG));
		if (precent == 100)
		{
			MineMiningSpeed ms;
			ms.uSize = sizeof(MineMiningSpeed);
			ms.uType = MineMsgType::MING_MINING_SPEED;
			ms.uRate = (uint64_t)0;
			MsgWndIPC::Instance()->WritePipeData((byte*)&ms, sizeof(MineMiningSpeed));
		}
		
	}
	return;
}

void MinerCLI::start()
{
	EthashGPUMiner::onDAGProgess([&](int _pc)
	{
		ProcessDagProgress(_pc);
		return;
	});
	EthashGPUMiner::setCheckQuitHandler([&]()
	{
		return MsgWndIPC::Instance()->WaitQuitEvent();
	});
	/*m_pfarm = new GenericFarm<EthashProofOfWork>();
	if (m_stratumClientVersion == 1)
	m_pClient = new EthStratumClient(m_pfarm, m_minerType, m_farmURL, m_port, m_user, m_pass, m_maxFarmRetries, m_worktimeout, m_stratumProtocol, m_email);
	else if (m_stratumClientVersion == 2)
	m_pClientV2 = new EthStratumClientV2(m_pfarm, m_minerType, m_farmURL, m_port, m_user, m_pass, m_maxFarmRetries, m_worktimeout, m_stratumProtocol, m_email);
	doStratum();*/
	doStratum();
}


void MinerCLI::doFarm(MinerType _m, string & _remote, unsigned _recheckPeriod)
{
	map<string, GenericFarm<EthashProofOfWork>::SealerDescriptor> sealers;
	sealers["cpu"] = GenericFarm<EthashProofOfWork>::SealerDescriptor{ &EthashCPUMiner::instances, [](GenericMiner<EthashProofOfWork>::ConstructionInfo ci){ return new EthashCPUMiner(ci); } };
#if ETH_ETHASHCL
	sealers["opencl"] = GenericFarm<EthashProofOfWork>::SealerDescriptor{ &EthashGPUMiner::instances, [](GenericMiner<EthashProofOfWork>::ConstructionInfo ci){ return new EthashGPUMiner(ci); } };
#endif
#if ETH_ETHASHCUDA
	sealers["cuda"] = GenericFarm<EthashProofOfWork>::SealerDescriptor{ &EthashCUDAMiner::instances, [](GenericMiner<EthashProofOfWork>::ConstructionInfo ci){ return new EthashCUDAMiner(ci); } };
#endif
	(void)_m;
	(void)_remote;
	(void)_recheckPeriod;
#if ETH_JSONRPC || !ETH_TRUE
	jsonrpc::HttpClient client(m_farmURL);
	::FarmClient rpc(client);
	jsonrpc::HttpClient failoverClient(m_farmFailOverURL);
	::FarmClient rpcFailover(failoverClient);

	FarmClient * prpc = &rpc;

	h256 id = h256::random();
	GenericFarm<EthashProofOfWork> f;
	f.setSealers(sealers);
	if (_m == MinerType::CPU)
		f.start("cpu", false);
	else if (_m == MinerType::CL)
		f.start("opencl", false);
	else if (_m == MinerType::CUDA)
		f.start("cuda", false);
	EthashProofOfWork::WorkPackage current, previous;
	boost::mutex x_current;
	EthashAux::FullType dag;
	while (m_running)
		try
	{
		bool completed = false;
		EthashProofOfWork::Solution solution;
		f.onSolutionFound([&](EthashProofOfWork::Solution sol)
		{
			solution = sol;
			return completed = true;
		});
		for (unsigned i = 0; !completed; ++i)
		{
			auto mp = f.miningProgress();
			f.resetMiningProgress();
			if (current)
				minelog << "Mining on PoWhash" << "#" + (current.headerHash.hex().substr(0, 8)) << ": " << mp << f.getSolutionStats();
			else
				minelog << "Getting work package...";

			auto rate = mp.rate();

			try
			{
				prpc->eth_submitHashrate(toJS((u256)rate), "0x" + id.hex());
			}
			catch (jsonrpc::JsonRpcException const& _e)
			{
				cwarn << "Failed to submit hashrate.";
				cwarn << boost::diagnostic_information(_e);
			}

			Json::Value v = prpc->eth_getWork();
			h256 hh(v[0].asString());
			h256 newSeedHash(v[1].asString());

			if (hh != current.headerHash)
			{
				x_current.lock();
				previous.headerHash = current.headerHash;
				previous.seedHash = current.seedHash;
				previous.boundary = current.boundary;
				current.headerHash = hh;
				current.seedHash = newSeedHash;
				current.boundary = h256(fromHex(v[2].asString()), h256::AlignRight);
				minelog << "Got work package: #" + current.headerHash.hex().substr(0, 8);
				f.setWork(current);
				x_current.unlock();
			}
			this_thread::sleep_for(chrono::milliseconds(_recheckPeriod));
		}
		cnote << "Solution found; Submitting to" << _remote << "...";
		cnote << "  Nonce:" << solution.nonce.hex();
		if (EthashAux::eval(current.seedHash, current.headerHash, solution.nonce).value < current.boundary)
		{
			bool ok = prpc->eth_submitWork("0x" + toString(solution.nonce), "0x" + toString(current.headerHash), "0x" + toString(solution.mixHash));
			if (ok) {
				cnote << "B-) Submitted and accepted.";
				f.acceptedSolution(false);
			}
			else {
				cwarn << ":-( Not accepted.";
				f.rejectedSolution(false);
			}
			//exit(0);
		}
		else if (EthashAux::eval(previous.seedHash, previous.headerHash, solution.nonce).value < previous.boundary)
		{
			bool ok = prpc->eth_submitWork("0x" + toString(solution.nonce), "0x" + toString(previous.headerHash), "0x" + toString(solution.mixHash));
			if (ok) {
				cnote << "B-) Submitted and accepted.";
				f.acceptedSolution(true);
			}
			else {
				cwarn << ":-( Not accepted.";
				f.rejectedSolution(true);
			}
			//exit(0);
		}
		else {
			f.failedSolution();
			cwarn << "FAILURE: GPU gave incorrect result!";
		}
		current.reset();
	}
	catch (jsonrpc::JsonRpcException&)
	{
		if (m_maxFarmRetries > 0)
		{
			for (auto i = 3; --i; this_thread::sleep_for(chrono::seconds(1)))
				cerr << "JSON-RPC problem. Probably couldn't connect. Retrying in " << i << "... \r";
			cerr << endl;
		}
		else
		{
			cerr << "JSON-RPC problem. Probably couldn't connect." << endl;
		}
		if (m_farmFailOverURL != "")
		{
			m_farmRetries++;
			if (m_farmRetries > m_maxFarmRetries)
			{
				if (_remote == "exit")
				{
					m_running = false;
				}
				else if (_remote == m_farmURL) {
					_remote = m_farmFailOverURL;
					prpc = &rpcFailover;
				}
				else {
					_remote = m_farmURL;
					prpc = &rpc;
				}
				m_farmRetries = 0;
			}

		}
	}
#endif
	exit(0);
}

void MinerCLI::doStratum()
{
	minelog << "doStratum";
	map<string, GenericFarm<EthashProofOfWork>::SealerDescriptor> sealers;
	sealers["cpu"] = GenericFarm<EthashProofOfWork>::SealerDescriptor{ &EthashCPUMiner::instances, [](GenericMiner<EthashProofOfWork>::ConstructionInfo ci){ return new EthashCPUMiner(ci); } };
#if ETH_ETHASHCL
	sealers["opencl"] = GenericFarm<EthashProofOfWork>::SealerDescriptor{ &EthashGPUMiner::instances, [](GenericMiner<EthashProofOfWork>::ConstructionInfo ci){ return new EthashGPUMiner(ci); } };
#endif
#if ETH_ETHASHCUDA
	sealers["cuda"] = GenericFarm<EthashProofOfWork>::SealerDescriptor{ &EthashCUDAMiner::instances, [](GenericMiner<EthashProofOfWork>::ConstructionInfo ci){ return new EthashCUDAMiner(ci); } };
#endif
	if (!m_farmRecheckSet)
		m_farmRecheckPeriod = m_defaultStratumFarmRecheckPeriod;

	m_pfarm = new GenericFarm<EthashProofOfWork>();
	// this is very ugly, but if Stratum Client V2 tunrs out to be a success, V1 will be completely removed anyway
	MsgWndIPC::Instance()->WaitForConnectPipe(); //检测是否状态
	if (m_stratumClientVersion == 1) {
		EthStratumClient client(m_pfarm, m_minerType, m_farmURL, m_port, m_user, m_pass, m_maxFarmRetries, m_worktimeout, m_stratumProtocol, m_email, [&](){return MsgWndIPC::Instance()->WaitQuitEvent(); });
		if (m_farmFailOverURL != "")
		{
			if (m_fuser != "")
			{
				client.setFailover(m_farmFailOverURL, m_fport, m_fuser, m_fpass);
			}
			else
			{
				client.setFailover(m_farmFailOverURL, m_fport);
			}
		}
		m_pfarm->setSealers(sealers);

		m_pfarm->onSolutionFound([&](EthashProofOfWork::Solution sol)
		{
			if (client.isConnected()) {
				client.submit(sol);

				MineSolutionFind msf;
				msf.uSize = sizeof(MineSolutionFind);
				msf.uType = MineMsgType::MING_SOLUTION_FIND;
				strcpy(msf.szNonce, toString(sol.nonce).c_str());
				strcpy(msf.szHeaderHash, toString(client.currentHeaderHash()).c_str());
				strcpy(msf.szMixHash, toString(sol.mixHash).c_str());
				MsgWndIPC::Instance()->WritePipeData((byte*)&msf, sizeof(MineSolutionFind));
			}
			else {
				cwarn << "Can't submit solution: Not connected";
			}
			return false;
		});

		while (client.isRunning())
		{
			//MsgWndIPC::Instance()->WaitForConnectPipe(); //检测是否状态
			auto mp = m_pfarm->miningProgress();
			m_pfarm->resetMiningProgress();
			if (client.isConnected())
			{
				if (client.current())
				{
					//minelog << "Mining on PoWhash" << "#" + (client.currentHeaderHash().hex().substr(0, 8)) << ": " << mp << m_pfarm->getSolutionStats();
					if (m_bCompleteDAGOnce || mp.rate() > 0)
					{
						minelog << "Mining on PoWhash" << "#" + (client.currentHeaderHash().hex().substr(0, 8)) << ": " << mp << m_pfarm->getSolutionStats();
						if (mp.rate() > 0)
						{
							MineMiningSpeed ms;
							ms.uSize = sizeof(MineMiningSpeed);
							ms.uType = MineMsgType::MING_MINING_SPEED;
							ms.uRate = (uint64_t)mp.rate();
							MsgWndIPC::Instance()->WritePipeData((byte*)&ms, sizeof(MineMiningSpeed));
						}
					}
				}
				else if (client.waitState() == MINER_WAIT_STATE_WORK)
					minelog << "Waiting for work package...";
			}
			this_thread::sleep_for(chrono::milliseconds(m_farmRecheckPeriod));
		}
	}
	else if (m_stratumClientVersion == 2) {
		EthStratumClientV2 client(m_pfarm, m_minerType, m_farmURL, m_port, m_user, m_pass, m_maxFarmRetries, m_worktimeout, m_stratumProtocol, m_email, [&](){return MsgWndIPC::Instance()->WaitQuitEvent(); });
		if (m_farmFailOverURL != "")
		{
			if (m_fuser != "")
			{
				client.setFailover(m_farmFailOverURL, m_fport, m_fuser, m_fpass);
			}
			else
			{
				client.setFailover(m_farmFailOverURL, m_fport);
			}
		}
		m_pfarm->setSealers(sealers);

		m_pfarm->onSolutionFound([&](EthashProofOfWork::Solution sol)
		{
			client.submit(sol);

			MineSolutionFind msf;
			msf.uSize = sizeof(MineSolutionFind);
			msf.uType = MineMsgType::MING_SOLUTION_FIND;
			strcpy(msf.szNonce, toString(sol.nonce).c_str());
			strcpy(msf.szHeaderHash, toString(client.currentHeaderHash()).c_str());
			strcpy(msf.szMixHash, toString(sol.mixHash).c_str());
			MsgWndIPC::Instance()->WritePipeData((byte*)&msf, sizeof(MineSolutionFind));
			return false;
		});

		while (client.isRunning())
		{
			//MsgWndIPC::Instance()->WaitWorkEvent(); //检测是否状态
			//MsgWndIPC::Instance()->WaitForConnectPipe(); //检测是否状态
			auto mp = m_pfarm->miningProgress();
			m_pfarm->resetMiningProgress();
			if (client.isConnected())
			{
				if (client.current())
				{
					//minelog << "Mining on PoWhash" << "#" + (client.currentHeaderHash().hex().substr(0, 8)) << ": " << mp << m_pfarm->getSolutionStats();
					if (m_bCompleteDAGOnce || mp.rate() > 0)
					{
						minelog << "Mining on PoWhash" << "#" + (client.currentHeaderHash().hex().substr(0, 8)) << ": " << mp << m_pfarm->getSolutionStats();
						if (mp.rate() > 0)
						{
							MineMiningSpeed ms;
							ms.uSize = sizeof(MineMiningSpeed);
							ms.uType = MineMsgType::MING_MINING_SPEED;
							ms.uRate = (uint64_t)mp.rate();
							MsgWndIPC::Instance()->WritePipeData((byte*)&ms, sizeof(MineMiningSpeed));
						}
					}
				}
				else if (client.waitState() == MINER_WAIT_STATE_WORK)
					minelog << "Waiting for work package...";
			}
			this_thread::sleep_for(chrono::milliseconds(m_farmRecheckPeriod));
		}
	}
	if (m_pfarm)
	{
		delete m_pfarm;
		m_pfarm = NULL;
	}
	minelog << "end do Stratum";
}