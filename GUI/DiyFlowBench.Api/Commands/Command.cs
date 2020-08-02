/****************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 ***/

// Development and release version - Don't forget to update the changelog!!
using System.Linq;
using System.Linq.Expressions;

namespace DiyFlowBench.Api.Commands
{
    /// <summary>
    /// A command that can be executed on the DIY Flow Bench controller.  See the API section of the WIKI for information on what commands are supported.
    /// </summary>
    public class Command
    {
        protected Command(char identifier)
        {
            if (!SupportedCommands.Contains(identifier)) throw new UnsupportedCommandIdentiferException(identifier);

            Identifier = identifier;
        }        

        /// <summary>
        /// The identifier of the command to be executed.  See the API section of the WIKI for a list of command identifiers.
        /// </summary>
        public char Identifier { get; private set; }

        private static char[] SupportedCommands { get { return new char[] {'V', 'F', 'T', 'H', 'R', 'B' }; } }
    }
}
