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
namespace DiyFlowBench.Api.Commands
{
    public class Command
    {
        protected Command(char identifier)
        {
            if (identifier < 0x21 || identifier > 0x7E) throw new UnsupportedCommandIdentiferException(identifier);

            Identifier = identifier;
        }        

        public char Identifier { get; private set; }
    }
}
