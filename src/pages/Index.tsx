import { useState } from "react";
import { motion } from "framer-motion";
import { Header } from "@/components/Header";
import { GameSelector } from "@/components/GameSelector";
import { StatsPanel } from "@/components/StatsPanel";
import { TiragesTable } from "@/components/TiragesTable";
import { DataImporter } from "@/components/DataImporter";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { BarChart3, Database, Upload } from "lucide-react";

export type GameType = "loto" | "euromillions" | "eurodreams" | "crescendo";

const Index = () => {
  const [selectedGame, setSelectedGame] = useState<GameType>("loto");
  const [refreshKey, setRefreshKey] = useState(0);

  const handleDataUpdate = () => {
    setRefreshKey((prev) => prev + 1);
  };

  return (
    <div className="min-h-screen bg-background">
      <Header />
      
      <main className="container mx-auto px-4 py-8">
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.5 }}
        >
          <GameSelector 
            selectedGame={selectedGame} 
            onGameChange={setSelectedGame} 
          />
        </motion.div>

        <Tabs defaultValue="stats" className="mt-8">
          <TabsList className="grid w-full max-w-md mx-auto grid-cols-3">
            <TabsTrigger value="stats" className="flex items-center gap-2">
              <BarChart3 className="h-4 w-4" />
              Statistiques
            </TabsTrigger>
            <TabsTrigger value="tirages" className="flex items-center gap-2">
              <Database className="h-4 w-4" />
              Tirages
            </TabsTrigger>
            <TabsTrigger value="import" className="flex items-center gap-2">
              <Upload className="h-4 w-4" />
              Import
            </TabsTrigger>
          </TabsList>

          <TabsContent value="stats" className="mt-6">
            <motion.div
              key={`stats-${selectedGame}-${refreshKey}`}
              initial={{ opacity: 0, x: -20 }}
              animate={{ opacity: 1, x: 0 }}
              transition={{ duration: 0.3 }}
            >
              <StatsPanel gameType={selectedGame} />
            </motion.div>
          </TabsContent>

          <TabsContent value="tirages" className="mt-6">
            <motion.div
              key={`tirages-${selectedGame}-${refreshKey}`}
              initial={{ opacity: 0, x: -20 }}
              animate={{ opacity: 1, x: 0 }}
              transition={{ duration: 0.3 }}
            >
              <TiragesTable gameType={selectedGame} />
            </motion.div>
          </TabsContent>

          <TabsContent value="import" className="mt-6">
            <motion.div
              initial={{ opacity: 0, x: -20 }}
              animate={{ opacity: 1, x: 0 }}
              transition={{ duration: 0.3 }}
            >
              <DataImporter 
                selectedGame={selectedGame} 
                onDataImported={handleDataUpdate} 
              />
            </motion.div>
          </TabsContent>
        </Tabs>
      </main>
    </div>
  );
};

export default Index;
