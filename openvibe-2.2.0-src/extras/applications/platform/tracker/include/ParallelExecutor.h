
#pragma once

#include <map>
#include <list>
#include <deque>
#include <vector>

#include <mutex>
#include <condition_variable>
#include <thread>

/*
 * Class allowing creating a set of worker threads that then pull jobs from a job list.
 *
 * Before this class can be used safely, various components in the OV SDK should be made 
 * thread safe. These are at least
 * - LogManager
 * - CKernelObjectFactory
 * - AlgorithmManager?
 * - ConfigurationManager? (if any plugin adds tokens)
 *
 */
namespace OpenViBETracker
{
	typedef std::function<void(size_t)> jobCall;

	// Fwd declare
	class CWorkerThread;

	class ParallelExecutor
	{
	public:
		ParallelExecutor() { };	
		bool initialize(const uint32_t nThreads);
		bool uninitialize();
		uint32_t getNumThreads(void) const { return (uint32_t)m_Threads.size(); }; 

		// Clients call these
		bool pushJob(const jobCall& someJob);                          // add job, pass to threads
		bool pushJobList(const std::deque<jobCall>& vJobList);         // add jobs
		bool clearPendingJobs(void);                                   // Delete all jobs thath haven't been launched yet      
		bool waitForAll(void);                                         // wait until all pushed jobs are done
		size_t getJobCount(void) const;                                // Number of remaining jobs
		bool isIdle(void) const;                                       // Nothing running & no tasks in list?

		// Worker threads call these
		bool getJob(jobCall& job);                                     // The call will hang until there is a job available or the executor is told to quit
		bool declareDone(void);                                        // Declare the previously obtained job as done

		bool launchTest(void);

	private:

		mutable std::mutex m_JobMutex;

		  std::deque<jobCall> m_JobList;
		  uint32_t m_nJobsRunning;

		  std::condition_variable m_HaveWork;
		  std::condition_variable m_JobDone;
	
		  bool m_Quit;

		std::vector<CWorkerThread*> m_WorkerThreads;
		std::vector<std::thread*> m_Threads;

	};

	// Provides the worker threads a limited access to the parallel executor
	class ExecutorView {
	public:
		ExecutorView(ParallelExecutor& exec) : m_Exec(exec) { };

		bool getJob(jobCall& job) { 
			return m_Exec.getJob(job); 
		}

		bool declareDone(void) {
			return m_Exec.declareDone();
		}

	private:
		ParallelExecutor& m_Exec;
	};

	class CWorkerThread {
	public:
		CWorkerThread() { };
	
		void run(ExecutorView ctx, uint32_t threadNumber)
		{
			while(true)
			{
				jobCall job;

				if(!ctx.getJob(job))
				{
					return;
				}

				// @todo pass job failed to caller
				job(threadNumber);

				ctx.declareDone();
			}
		}

		static void startWorkerThread(CWorkerThread* pThread, ExecutorView ctx, uint32_t threadNumber)
		{
			pThread->run(ctx, threadNumber);
		}
	};

};

