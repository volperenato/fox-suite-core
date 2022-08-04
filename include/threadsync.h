#ifndef __inc_ThreadSynch__
#define __inc_ThreadSynch__

namespace Fox
{
	class CriticalSection
	{
		public:
			CriticalSection() { InitializeCriticalSection(&cs); }
			~CriticalSection() { DeleteCriticalSection(&cs); }
			void Lock() { EnterCriticalSection(&cs); }
			void Release() { LeaveCriticalSection(&cs);	}
			operator CRITICAL_SECTION&() { return cs; }
		private:
			CRITICAL_SECTION cs;
	};

	class AutoLock
	{
		public:
			AutoLock(CriticalSection& cs) : _cs(cs) { _cs.Lock(); }	
			~AutoLock()	{ _cs.Release(); }
		private:
			CriticalSection& _cs;
	};
}

#
#endif