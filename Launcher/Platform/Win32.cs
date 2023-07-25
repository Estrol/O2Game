using System;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace Launcher.Platform {
    internal class Win32 {
        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        private static extern IntPtr LoadLibrary(string libname);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi)]
        private static extern bool FreeLibrary(IntPtr hModule);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procname);

        // "Game.dll"+local_main
        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        private delegate int local_main(int argc, string[] argv);

        public int Run(string[] args) {
            int ret_value = 0;

            IntPtr hModule = LoadLibrary("Game.dll");
            if (hModule == IntPtr.Zero) {
                MessageBox.Show(
                    "Failed to load library 'Game.dll' (error code: " + Marshal.GetLastWin32Error() + ")", 
                    "Error", 
                    MessageBoxButtons.OK, 
                    MessageBoxIcon.Error);

                return -1;
            }

            IntPtr pLocalMain = GetProcAddress(hModule, "local_main");
            if (pLocalMain == IntPtr.Zero) {
                MessageBox.Show(
                    "Game.dll does not contain EntryPoint! (error code: " + Marshal.GetLastWin32Error() + ")",
                    "Error",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);

                return -1;
            }

            local_main lm = (local_main)Marshal.GetDelegateForFunctionPointer(pLocalMain, typeof(local_main));
            ret_value = lm(args.Length, args);

            FreeLibrary(hModule);

            return ret_value;
        }
    }
}
